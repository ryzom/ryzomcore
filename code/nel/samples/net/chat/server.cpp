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

#include <string>

#include "nel/misc/common.h"
#include "nel/misc/path.h"

// contains the service base class
#include "nel/net/service.h"
#include "nel/net/callback_server.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;

#ifndef CHAT_DIR
#	define CHAT_DIR ""
#endif

// THE SERVER

// Must be a pointer to control when to start listening socket and when stop it
CCallbackServer *Server;
vector<TSockId> Clients;

// MESSAGES

// ***************************************************************************
void clientWantsToConnect ( TSockId from, void *arg )
{
	// Called when a client wants to connect
	Clients.push_back (from);
}

// ***************************************************************************
void clientWantsToDisconnect ( TSockId from, void *arg )
{
	// Called when a client wants to disconnect
	for (uint i = 0; i < Clients.size(); ++i)
		if (Clients[i] == from)
		{
			Clients.erase(Clients.begin()+i);
			return;
		}
}

// ***************************************************************************
void clientSentChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when a client sent a CHAT message
	string text;
	msgin.serial(text);
	CMessage msgout;
	msgout.setType("CHAT");
	msgout.serial(text);
	for (uint i = 0; i < Clients.size(); ++i)
		Server->send(msgout, Clients[i]);
}

// ***************************************************************************
// All messages handled by this server
#define NB_CB 1
TCallbackItem CallbackArray[NB_CB] =
{
	{ "CHAT", clientSentChat }
};

// SERVICE

// ***************************************************************************
class CChatService : public IService
{
public:

	void init ()
	{
		// Init the server on port 3333
		Server = new CCallbackServer();
		Server->init (3333);
		Server->setConnectionCallback (clientWantsToConnect, NULL);
		Server->setDisconnectionCallback (clientWantsToDisconnect, NULL);
		Server->addCallbackArray (CallbackArray, NB_CB);
	}

	bool update ()
	{
		// this function is called every "loop". you return true if you want
		// to continue or return false if you want to exit the service.
		// the loop is called evenly (by default, at least one time per second).

		Server->update();

		return true;
	}

	void release ()
	{
		// Must delete the server here
		delete Server;
	}
};

// this macro is the "main". the first param is the class name inherited from IService.
// the second one is the name of the service used to register and find the service
// using the naming service. the third one is the port where the listen socket will
// be created. If you put 0, the system automatically finds a port.
NLNET_SERVICE_MAIN (CChatService, "CS", "chat_service", 0, EmptyCallbackArray, CHAT_DIR, "");
