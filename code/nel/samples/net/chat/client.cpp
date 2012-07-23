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
 * Login system example, client.
 *
 * This client connects to a front-end server using login system.
 *
 * Before running this client, the front end service sample must run,
 * and also the NeL naming_service, time_service, login_service, welcome_service.
 *
 */

//
// Includes
//

#include <string>

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"
#include "nel/misc/path.h"

#include "nel/net/callback_client.h"

#ifdef NL_OS_WINDOWS
#include <conio.h>
#else
#include "kbhit.h"
#endif

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// Really simple chat
// Do not display what you are typing

#ifdef NL_OS_WINDOWS
#define KEY_ESC		27
#define KEY_ENTER	13
#else
#define KEY_ESC		27
#define KEY_ENTER	10
#endif

#ifndef CHAT_DIR
#	define CHAT_DIR "."
#endif

string CurrentEditString;

// ***************************************************************************
void serverSentChat (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	// Called when the server sent a CHAT message
	string text;
	msgin.serial(text);
	printf("%s\n", text.c_str());
}

// ***************************************************************************
// All messages handled by this server
#define NB_CB 1
TCallbackItem CallbackArray[NB_CB] =
{
	{ "CHAT", serverSentChat }
};

/*
 * main
 */
int main (int argc, char **argv)
{
	NLMISC::CApplicationContext applicationContext;

	CCallbackClient *Client;

	NLMISC::CPath::addSearchPath(CHAT_DIR);

	// Read the host where to connect in the client.cfg file
	CConfigFile ConfigFile;	
	ConfigFile.load (NLMISC::CPath::lookup("client.cfg"));
	string LSHost(ConfigFile.getVar("LSHost").asString());

	// Init and Connect the client to the server located on port 3333
	Client = new CCallbackClient();
	Client->addCallbackArray (CallbackArray, NB_CB);

	printf("Please wait connecting...\n");
	try
	{
		CInetAddress addr(LSHost+":3333");
		Client->connect(addr);
	}
	catch(const ESocket &e)
	{
		printf("%s\n", e.what());
		return 0;
	}

	if (!Client->connected())
	{
		printf("Connection Error\n");
		return 0;
	}
	printf("Connected.\n");

#ifdef NL_OS_UNIX
	init_keyboard();
#endif
	// The main loop
	do
	{
		int c;
		if (kbhit())
		{
			c = getch();
			if (c == KEY_ESC)
			{
				break; // FINSIH
			}
			else if (c == KEY_ENTER)
			{
				CMessage msg;
				msg.setType("CHAT");
				msg.serial (CurrentEditString);
				Client->send(msg);
				CurrentEditString = "";
			}
			else
			{
				CurrentEditString += c;
			}
		}
		Client->update();
	}
	while (Client->connected());

#ifdef NL_OS_UNIX
	close_keyboard();
#endif
	// Finishing
	delete Client;
}
