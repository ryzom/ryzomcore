// NeLNS - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#include <stdio.h>
#include <ctype.h>
#include <math.h>

#include <vector>
#include <map>

#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/displayer.h"
#include "nel/misc/log.h"

#include "nel/net/buf_server.h"
#include "nel/net/service.h"

#include "admin_service.h"

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Variables
//

CBufServer *WebServer = 0;


//
// Callbacks
//

void cbGetRequest (CMemStream &msgin, TSockId host)
{
	string rawvarpath;
	msgin.serial (rawvarpath);
	addRequest (rawvarpath, host);
}

typedef void (*WebCallback)(CMemStream &msgin, TSockId host);

WebCallback WebCallbackArray[] =
{
	cbGetRequest,
};

//
// Functions
//

void sendString (TSockId from, const string &str)
{
	nlinfo ("REQUEST: Send '%s' to php '%s'", str.c_str (), from->asString ().c_str());

	if(from == 0)
		return;

	CMemStream msgout;
	uint32 fake = 0;
	msgout.serial(fake);
	msgout.serial (const_cast<string&>(str));
	WebServer->send (msgout, from);
}

void connectionWebInit ()
{
	nlassert(WebServer == 0);

	WebServer = new CBufServer ();
	nlassert(WebServer != 0);

	uint16 port = (uint16) IService::getInstance ()->ConfigFile.getVar ("WebPort").asInt();
	WebServer->init (port);

	nlinfo ("Set the server connection for web to port %hu", port);
}

void connectionWebUpdate ()
{
	nlassert(WebServer != 0);

	try
	{
		WebServer->update ();

		while (WebServer->dataAvailable ())
		{
			// create a string mem stream to easily communicate with web server
			NLMISC::CMemStream msgin (true);
			TSockId host;
			sint8 messageType = 0;

			try
			{
				WebServer->receive (msgin, &host);
				uint32 fake = 0;
				msgin.serial(fake);

				msgin.serial (messageType);
			}
			catch (Exception &e)
			{
				nlwarning ("Error during receive: '%s'", e.what ());
			}

			if(messageType>=0 && messageType<(sint8)(sizeof(WebCallbackArray)/sizeof(WebCallbackArray[0])))
			{
				WebCallbackArray[messageType](msgin, host);
			}
			else
			{
				nlwarning ("Received an unknown message type %d from web server", messageType);
			}
		}
	}
	catch (Exception &e)
	{
		nlwarning ("Error during update: '%s'", e.what ());
	}
}

void connectionWebRelease ()
{
	nlassert(WebServer != 0);

	delete WebServer;
	WebServer = 0;
}
