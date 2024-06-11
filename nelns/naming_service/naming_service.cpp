// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2023  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#ifndef NELNS_CONFIG
#define NELNS_CONFIG ""
#endif // NELNS_CONFIG

#ifndef NELNS_LOGS
#define NELNS_LOGS ""
#endif // NELNS_LOGS


//
// Includes
//

#include <nel/misc/types_nl.h>

#include <list>
#include <string>

#include <nel/misc/debug.h>
#include <nel/misc/command.h>
#include <nel/misc/variable.h>
#include <nel/misc/displayer.h>

#include <nel/net/service.h>

#include <nelns/naming_service/functions.h>
#include <nelns/naming_service/naming_service.h>
#include <nelns/naming_service/service_entry.h>
#include <nelns/naming_service/service_instance_manager.h>
#include <nelns/naming_service/variables.h>

//
// Namespaces
//

using namespace std;

using namespace NLMISC;
using namespace NLNET;


NLMISC_COMMAND(test, "none", "none")
{
	log.displayNL("Raw cmd line : '%s'", rawCommandString.c_str());
	log.displayNL("Dumping %u parameters :", args.size());
	for (uint i=0; i<args.size(); ++i)
	{
		log.displayNL("  %u : '%s'", i, args[i].c_str());
	}
	return true;
}


static const char* getCompleteServiceName(const IService* theService)
{
	static std::string s;
	s= "naming_service";

	if (theService->haveLongArg("nsname"))
	{
		s+= "_"+theService->getLongArg("nsname");
	}

	if (theService->haveLongArg("fullnsname"))
	{
		s= theService->getLongArg("fullnsname");
	}

	return s.c_str();
}

static const char* getShortServiceName(const IService* theService)
{
	static std::string s;
	s= "NS";

	if (theService->haveLongArg("shortnsname"))
	{
		s= theService->getLongArg("shortnsname");
	}
	
	return s.c_str();
}
//
/// Naming Service
//
NLNET_SERVICE_MAIN( CNamingService, getShortServiceName(scn), getCompleteServiceName(scn), 0, EmptyCallbackArray, NELNS_CONFIG, NELNS_LOGS)


//
// Commands
//


NLMISC_COMMAND (nsServices, "displays the list of all registered services", "")
{
	if(args.size() != 0) return false;

	displayRegisteredServices (&log);

	return true;
}

NLMISC_COMMAND (kill, "kill a service and send an unregister broadcast to other service", "<ServiceShortName>|<ServiceId>")
{
	if(args.size() != 1) return false;

	// try with number

	TServiceId sid(atoi(args[0].c_str()));

	if(sid.get() == 0)
	{
		// not a number, try a name
		list<CServiceEntry>::iterator it;
		for (it = RegisteredServices.begin(); it != RegisteredServices.end (); it++)
		{
			if ((*it).Name == args[0])
			{
				sid = (*it).SId;
				break;
			}
		}
		if (it == RegisteredServices.end())
		{
			log.displayNL ("Bad service name or id '%s'", args[0].c_str());
			return false;
		}
	}

	doUnregisterService (sid);
	return true;
}

NLMISC_DYNVARIABLE(uint32, NbRegisteredServices, "display the number of service that are registered in naming service")
{
	if (get) *pointer = (uint32)RegisteredServices.size();
}

NLMISC_COMMAND( displayServiceInstances, "SIM: Display info on service instances", "" )
{
	CServiceInstanceManager::getInstance()->displayInfo( &log );
	return true;
}

NLMISC_COMMAND( killAllServices, "SIM: Make all the controlled services quit", "" )
{
	CServiceInstanceManager::getInstance()->killAllServices();
	return true;
}
