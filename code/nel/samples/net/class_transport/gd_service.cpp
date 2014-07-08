// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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


/*
 * Transport class example, gd server.
 *
 * This service have a class (CSharedClass) that send to other service when online.
 *
 * To run this program, ensure there is a file "gd_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory. The naming service must be running.
 */

//
// Includes
//

// We're using the NeL Service framework, and layer 5
#include "nel/net/service.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/displayer.h"

#include "nel/net/transport_class.h"

#ifdef NL_OS_WINDOWS
#	ifndef NL_COMP_MINGW
#		define NOMINMAX
#	endif
#	include <windows.h>
#endif // NL_OS_WINDOWS

#ifndef NL_CT_CFG
#define NL_CT_CFG ""
#endif // NL_CT_CFG

//
// Namespace
//

using namespace std;
using namespace NLNET;
using namespace NLMISC;

//
// Shared Class
//

struct CSharedClass : public CTransportClass
{
	uint32	i1, i2, i3;
	float	f1;

	vector<uint32> vi1;

	string str;

	CEntityId eid;

	CSharedClass () : i1(10), i2(10), i3(10), f1(10), str("str10") { vi1.push_back(111); vi1.push_back(222); vi1.push_back(255); }

	virtual void description ()
	{
		className ("SharedClass");
		property ("i1", PropUInt32, (uint32)11, i1);
		property ("f1", PropFloat,  1.5f, f1);
//		property ("i2", PropUInt32, (uint32)12, i2);
		property ("i3", PropUInt32, (uint32)13, i3);
		propertyCont ("vi1", PropUInt32, vi1);
		property ("str", PropString, (string)"str12", str);
//		property ("eid", PropEntityId, CEntityId::Unknown, eid);
	}

	virtual void callback (const string &name, uint8 sid)
	{
		nlinfo ("Yes! I receive a Shared class from '%s' %d", name.c_str(), sid);
	}
};

//
// Variables
//

static CSharedClass SharedClass;

//
// Functions
//

static void cbUpService (const std::string &serviceName, uint16 sid, void *arg)
{
	// When a service comes, send the new class
	CSharedClass foo;
	foo.send((TServiceId)sid);
}

//
// Service class
//

struct CGDService : public IService
{
	void init()
	{
		// callback when a new service comes
		CUnifiedNetwork::getInstance()->setServiceUpCallback("*", (TUnifiedNetCallback)cbUpService, NULL);

		// init the class transport system
		CTransportClass::init ();

		// register the shared class
		TRANSPORT_CLASS_REGISTER (CSharedClass);
	}

	void release ()
	{
		// release all the class transport system
		CTransportClass::release ();
	}
};


/*
 * Declare a service with the class IService, the names "GDS" (short) and "gd_service" (long).
 * The port is automatically allocated (0) and the main callback array is empty.
 */
NLNET_SERVICE_MAIN( CGDService, "GDS", "gd_service", 0, EmptyCallbackArray, NL_CT_CFG, "" )
