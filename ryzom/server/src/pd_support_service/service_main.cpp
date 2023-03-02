// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2021  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

//-----------------------------------------------------------------------------
// include
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/common.h"
#include "nel/net/service.h"

// game share
#include "game_share/ryzom_version.h"
#include "game_share/tick_event_handler.h"
#include "game_share/singleton_registry.h"
#include "server_share/handy_commands.h"

// local
#include "service_main.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

//-----------------------------------------------------------------------------
// force admin module to link in
//-----------------------------------------------------------------------------

extern void admin_modules_forceLink();
void foo()
{
	admin_modules_forceLink();
}


//-----------------------------------------------------------------------------
// namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//-----------------------------------------------------------------------------
// methods CServiceClass
//-----------------------------------------------------------------------------

void CServiceClass::init()
{
	setVersion (RYZOM_PRODUCT_VERSION);

	// if we are connecting to a shard then start by initializing the tick interface
	if (IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")==NULL || IService::getInstance()->ConfigFile.getVarPtr("DontUseTS")->asInt()==0)
	{
		bool useMirror = (IService::getInstance()->ConfigFile.getVarPtr("UseMirror")!=NULL && IService::getInstance()->ConfigFile.getVarPtr("UseMirror")->asBool());
		if(useMirror)
		{
			nlwarning("Using the mirror to provide ticks to the service - ignoring state of the DontUseTS variable");
		}
		else
		{
			CTickEventHandler::init(CServiceClass::tickUpdate);
		}
}

	CSingletonRegistry::getInstance()->init();
}

bool CServiceClass::update()
{
	CSingletonRegistry::getInstance()->serviceUpdate();
	return true;
}

void CServiceClass::tickUpdate()
{
	CSingletonRegistry::getInstance()->tickUpdate();
}

void CServiceClass::release()
{
	CSingletonRegistry::getInstance()->release();
}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------

static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s;
	s= "pd_support_service";

	if (theService->haveLongArg("name"))
	{
		s+= "_"+theService->getLongArg("name");
	}

	if (theService->haveLongArg("fullname"))
	{
		s= theService->getLongArg("fullname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s;
	s= "PDSS";

	if (theService->haveLongArg("shortname"))
	{
		s= theService->getLongArg("shortname");
	}

	return s.c_str();
}

NLNET_SERVICE_MAIN( CServiceClass, getShortServiceName(scn), getCompleteServiceName(scn), 0, EmptyCallbackArray, "", "" );

