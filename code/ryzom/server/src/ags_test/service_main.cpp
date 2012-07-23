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
#include "nel/net/service.h"
#include "nel/misc/path.h"
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"

#include "messages.h"		// singleton manager for transport class messages
#include "mirrors.h"		// singleton manager for data in mirrors
#include "sheets.h"			// singleton manager for data from george forms
#include "move_manager.h"	// singleton manager for agent movement
#include "command_event_manager.h"

//#include "combat_interface.h"
//#include "bot_chat_interface.h"

#include "actor_manager.h"	// actor manager - the back end of the service

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace AGS_TEST;


static TUnifiedCallbackItem CallbackArray[] = 
{
	{	"QWERTY",			0,	},
};

/*-----------------------------------------------------------------*\
						SERVICE CLASS
\*-----------------------------------------------------------------*/

class CAgsTest : public NLNET::IService
{
public:
	void init();
	bool update();
	void release() {}
	void tickRelease();
};

// callback for the 'tick' update message
static void cbTick();

// callback for the 'tick' release message
void cbTickRelease()
{
	(static_cast<CAgsTest*>(CAgsTest::getInstance()))->tickRelease();
}

/*-----------------------------------------------------------------*\
						SERVICE INIT & RELEASE
\*-----------------------------------------------------------------*/

///init

void CAgsTest::init (void)
{
	setVersion (RYZOM_VERSION);

	// setup the update systems
//	setUpdateTimeout(200);

	// Load the sheet id lookup table

	// init sub systems
	CSheets::init();
	CMirrors::init(cbTick, NULL, cbTickRelease);
	CMessages::init();
	CMoveManager::init();
	CActorManager::init();
	//CCombatInterface::init();
	//CBotChatInterface::init();
	CCommandEventManager::init();

	// setup the debug filters
	DebugLog->addNegativeFilter("NETL");
}


///release

void CAgsTest::tickRelease (void)	
{
	// release sub systems
	CSheets::release();
	CMessages::release();
	CMoveManager::release();
	CActorManager::release();
	//CCombatInterface::release();
	//CBotChatInterface::release();
	CCommandEventManager::release();
	CMirrors::release();
}


/*-----------------------------------------------------------------*\
						SERVICE UPDATES
\*-----------------------------------------------------------------*/

///update called on each 'tick' message from tick service

void cbTick()
{
	if ( CMirrors::Mirror.mirrorIsReady() )
	{
		CActorManager::update();
		CMoveManager::update();
		//CCombatInterface::_events.clear();
		CCommandEventManager::update();
	}
}


///update called every complete cycle of service loop

bool CAgsTest::update (void)
{
	return true;
}

/*-----------------------------------------------------------------*\
						NLNET_SERVICE_MAIN
\*-----------------------------------------------------------------*/
NLNET_SERVICE_MAIN (CAgsTest, "AGS", "ags_test_service", 0, CallbackArray, "", "")

