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

#include "nel/ligo/primitive.h"
#include "nel/ligo/ligo_config.h"
#include "game_share/tick_event_handler.h"
#include "game_share/ryzom_version.h"

#include "world_container.h"
#include "ai.h"
#include "ai_mgr.h"
#include "ai_keywords.h"
#include "mirrors.h"
#include "messages.h"
#include "ais_actions.h"
#include "egs_interface.h"
#include "aids_interface.h"
#include "game_share/fame.h"
#include "visual_properties_interface.h"

#include "ais_user_models.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

//#include "nel/misc/bitmap.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

// force admin module to link in
extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}

bool EGSHasMirrorReady = false;
bool IOSHasMirrorReady = false;

// The ligo config
NLLIGO::CLigoConfig LigoConfig;

namespace AICOMP
{
	bool compileExternalScript (const char *filename, const char *outputFilename);
}

/*-----------------------------------------------------------------*\
						SERVICE CLASS
\*-----------------------------------------------------------------*/

class CAIService : public NLNET::IService
{
public:
	void commandStart();
	void init();
	bool update();
	void release();
	void tickRelease();
};

// callback for the 'tick' update message
void cbTick();

// callback for the 'tick' release message
void cbTickRelease()
{
	(static_cast<CAIService*>(CAIService::getInstance()))->tickRelease();
}

/*-----------------------------------------------------------------*\
						SERVICE INIT & RELEASE
\*-----------------------------------------------------------------*/


static void cbServiceMirrorUp( const std::string& serviceName, NLNET::TServiceId serviceId, void * )
{
	if	(serviceName == "IOS")
	{
		IOSHasMirrorReady = true;
	}

	if	(serviceName == "EGS")
	{
		EGSHasMirrorReady = true;

		// force an AI update on EGS up
		CAIS::instance().update();

		//send custom data to EGS
		CAIUserModelManager::getInstance()->sendUserModels();
		CAIUserModelManager::getInstance()->sendCustomLootTables();
	}

	CAIS::instance().serviceEvent(CServiceEvent(serviceId, serviceName, CServiceEvent::SERVICE_UP));
}

static void cbServiceDown( const std::string& serviceName, NLNET::TServiceId serviceId, void * )
{
	if ( serviceName == "IOS" )
	{
		IOSHasMirrorReady = false;
	}

	if ( serviceName == "EGS" )
	{
		EGSHasMirrorReady = false;
	}

	CAIS::instance().serviceEvent(CServiceEvent(serviceId, serviceName, CServiceEvent::SERVICE_DOWN));

}

void CAIService::commandStart()
{
	// Compile an AI script ?
	if (haveArg ('c'))
	{
		string scriptFilename = getArg ('c');
		bool result = false;
		if (!scriptFilename.empty ())
		{
			string outputFilename;
			if (haveArg ('o'))
				outputFilename = getArg ('o');

			// Compile and exit
			result = AICOMP::compileExternalScript (scriptFilename.c_str (), outputFilename.c_str ());
		}
		else
			nlwarning ("No script filename");

		::exit (result?0:-1);
	}
}

///init
void CAIService::init (void)
{
	// start any available system command.
	CConfigFile::CVar *sysCmds = IService::getInstance()->ConfigFile.getVarPtr("SystemCmd");
	if (sysCmds != NULL)
	{
		for (uint i=0; i<sysCmds->size(); ++i)
		{
			string cmd = sysCmds->asString(i);

			nlinfo("Invoking system command '%s'...", cmd.c_str());
			int ret = system(cmd.c_str());
			nlinfo(" command returned %d", ret);
		}
	}

	// read sheet_id.bin and don't prune out unknown files
	CSheetId::init(false);

    // Init singleton manager
	CSingletonRegistry::getInstance()->init();

	// init static fame manager
	CStaticFames::getInstance();

	setVersion (RYZOM_VERSION);

	// Init ligo
	if (!LigoConfig.readPrimitiveClass ("world_editor_classes.xml", false))
	{
		// Should be in l:\leveldesign\world_editor_files
		nlerror ("Can't load ligo primitive config file world_editor_classes.xml");
	}


	// have ligo library register its own class types for its class factory
	NLLIGO::Register();

	// setup the update systems
	setUpdateTimeout(100);



	// init sub systems
	CAIKeywords::init();
	CMirrors::init(cbTick, NULL, cbTickRelease);
	CMessages::init();
	AISHEETS::CSheets::getInstance()->init();

	// initialise the AI_SHARE library
	AI_SHARE::init(&LigoConfig);

	// set the primitive context
	NLLIGO::CPrimitiveContext::instance().CurrentLigoConfig = &LigoConfig;

	CAISActions::init();

	CEGSInterface::init();
	CTimeInterface::init();
	CCombatInterface::init();
	CVisualPropertiesInterface::init();
	CAIDSInterface::init();

	// register the service up and service down callbacks
	CMirrors::Mirror.setServiceMirrorUpCallback("*", cbServiceMirrorUp, 0);
	CMirrors::Mirror.setServiceDownCallback( "*", cbServiceDown, 0);
//	CUnifiedNetwork::getInstance()->setServiceDownCallback( "*", cbServiceDown, 0);

	CConfigFile::CVar	*clientCreature=IService::getInstance()->ConfigFile.getVarPtr ("CreatureDebug");
	if (clientCreature)
	{
		CAIS::instance().setClientCreatureDebug(clientCreature->asInt()!=0);
	}
}


///release

void CAIService::release()
{
}


void CAIService::tickRelease (void)
{
	CWorldContainer::clear();

	// release sub systems
	CAIS::instance().release();

	CAIDSInterface::release();
	CVisualPropertiesInterface::release();
	CCombatInterface::release();
	CTimeInterface::release();
	CEGSInterface::release();

	CAISActions::release();

	AISHEETS::CSheets::getInstance()->release();
	CMessages::release();

	CMirrors::release();

	CFamilyProfileFactory::instance().release();

	CSingletonRegistry::getInstance()->release();
}

void dispatchEvents()
{
	H_AUTO(dispatchEvents);
	while (!CCombatInterface::_events.empty())
	{
		CAIEntityPhysical	*target=CAIS::instance().getEntityPhysical(CCombatInterface::_events.front()._targetRow);
		if (target)
			target->processEvent( CCombatInterface::_events.front() );
		CCombatInterface::_events.pop_front();
	}
}

/*-----------------------------------------------------------------*\
						SERVICE UPDATES
\*-----------------------------------------------------------------*/

///update called on each 'tick' message from tick service

void cbTick()
{
	// cleanup stat variables
//	StatCSpawnBotFauna	 = 0;
//	StatCSpawnBotNpc	 = 0;
//	StatCBotPet			 = 0;
//	StatCAIContinent	 = 0;
//	StatCSpawnGroupFauna = 0;
//	StatCSpawnGroupNpc	 = 0;
//	StatCSpawnGroupPet	 = 0;
//	StatCAIInstance		 = 0;
//	StatCMgrFauna		 = 0;
//	StatCMgrNpc			 = 0;
//	StatCMgrPet			 = 0;
//	StatCBotPlayer		 = 0;
//	StatCPlayerManager	 = 0;
//	StatCcontinent		 = 0;
//	StatCRegion			 = 0;
//	StatCCellZone		 = 0;

	if ( CMirrors::mirrorIsReady() )
	{
		CAIS::instance().update();

		CEGSInterface::update();
		CTimeInterface::update();
		CVisualPropertiesInterface::update();

		dispatchEvents();

		CMirrors::update();
	}
	{
		H_AUTO(CSingletonRegistry_tickUpdate)
			CSingletonRegistry::getInstance()->tickUpdate();
	}
}



//prototype for a global routine in commands.cpp
void UpdateWatches();

uint ForceTicks=0;

///update called every coplete cycle of service loop
bool CAIService::update (void)
{
//	NLMEMORY::CheckHeap(true);

	UpdateWatches();
//	NLMEMORY::CheckHeap(true);

	if (ForceTicks)
	{
		// We update a false perception (as if we force ticks, it meens we don't receive true one because we're off-line).

		// We make a false tick.
		ForceTicks--;
		cbTick();
	}

	CSingletonRegistry::getInstance()->serviceUpdate();

	return true;
}



/*-----------------------------------------------------------------*\
						NLNET_SERVICE_MAIN
\*-----------------------------------------------------------------*/
NLNET_SERVICE_MAIN (CAIService, "AIS", "ai_service", 0, EmptyCallbackArray, "", "")

