/** \file tick_service.cpp
 * The TICK SERVICE deals with time management in the shard
 *
 * $Id: test.cpp,v 1.4 2005/04/25 23:55:00 miller Exp $
 */



#include "nel/misc/command.h"
#include "nel/misc/variable.h"
#include "nel/misc/common.h"
#include "nel/misc/file.h"

#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/singleton_registry.h"
#include "game_share/handy_commands.h"

#include "test.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;


/*
 * Initialise the service
 */
void CTest::init()
{
	setVersion (RYZOM_VERSION);

	// if we are connecting to a shard then start by initializing the tick interface
	if (IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")==NULL || IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")->asInt()==0)
		CTickEventHandler::init(CTest::tickUpdate);

	CSingletonRegistry::getInstance()->init();
}

/*
 * Update
 */
bool CTest::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	return true;
}

/*
 * Update
 */
void CTest::tickUpdate()
{
	CSingletonRegistry::getInstance()->tickUpdate();
}

/*
 * Release
 */
void CTest::release()
{
	CSingletonRegistry::getInstance()->release();
}

//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CTest, "TEST", "test_service", 0, EmptyCallbackArray, "", "" );

