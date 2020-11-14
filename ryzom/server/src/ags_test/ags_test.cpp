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



// Nel Misc
#include "nel/misc/command.h"
#include "nel/misc/path.h"

// Game share
#include "game_share/container_property_receiver.h"
#include "game_share/msg_brick_service.h"

// Local includes
#include "ags_test.h"
#include "actor.h"
#include "actor_manager.h"

#include "../src/agent_game/combat_interface.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;
using namespace AGS_TEST;


CAgsTest* pAgsTest = NULL;

/**
 *	CallbackArray
 */
TUnifiedCallbackItem CallbackArray[] = 
{
	{	"AZERTY",	NULL	},
};

//---------------------------------------------------
// Service Init :
// 
//---------------------------------------------------
void CAgsTest::init (void)
{
	// Load the sheet id lookup table

	// Initialise the actor manager
	CActorManager::init();

	// For windows speed update
	setUpdateTimeout(100);

	// keep the pointer on class if need use a methode
	pAgsTest = this;

	// init the Tick Event Handler
	CTickEventHandler::init( serviceUpdate, cbSync );

	// initialise the property receiver set
	CContainerPropertyReceiver::initPropertyReceiver();

	// init the combat interface
	CCombatInterface::init();
	
	// Init callback for service up / down
	CUnifiedNetwork::getInstance()->setServiceUpCallback ("*", cbServiceUp, NULL);
	CUnifiedNetwork::getInstance()->setServiceDownCallback( string("*"), cbServiceDown, 0);
}

//---------------------------------------------------
// Service update :
// 
//---------------------------------------------------
bool CAgsTest::update (void)
{
	return true;
}

//---------------------------------------------------
// Service release :
// 
//---------------------------------------------------
void CAgsTest::release (void)	
{
	CContainerPropertyReceiver::releaseAllPropertiesManagers();
}

//---------------------------------------------------
// Service update : Call at every ticks
// 
//---------------------------------------------------
void CAgsTest::serviceUpdate(void)
{
	CActorManager::update();
}

/****************************************************************\
 ****************************************************************
						Callback functions
 ****************************************************************
\****************************************************************/
// Callback called at service connexion
void cbServiceUp( const string& serviceName, uint16 serviceId, void * )
{
	/*if (serviceName==std::string("EGS"))
		CActorManager::reconnectEGS((uint8)serviceId);

	if (serviceName==std::string("IOS"))
		CActorManager::reconnectIOS((uint8)serviceId);

	if (serviceName==std::string("GPMS"))
		CActorManager::reconnectGPMS((uint8)serviceId);*/
}

// Callback called at service down
void cbServiceDown( const string& serviceName, uint16 serviceId, void * )
{
}

//---------------------------------------------------
// cbSync, for service init with current time/tick if needed
// 
//---------------------------------------------------
void cbSync()
{
} // cbSync //


/****************************************************************\
 ****************************************************************
						Service register
 ****************************************************************
\****************************************************************/
NLNET_SERVICE_MAIN (CAgsTest, "AgG", "ags_test_service", 0, CallbackArray, "", "")



//-------------------------------------------------------------------------
