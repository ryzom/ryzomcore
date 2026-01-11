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
#include "nel/misc/system_info.h"

// contains the service base class
#include "nel/net/service.h"

using namespace std;
using namespace NLMISC;
using namespace NLNET;

// a service is a process, a program. the goal is to automatically use
// common initialization and features (like command line, file check,
// debug features, signal redirection and more).

// a service has a listen socket for external connection. when a message
// comes on the socket, the message is automatically updated and the
// associated callback is called.

// to create a service, you have to inherit from IService and to implement
// a few functions (init, update, release). you are not forced to implement
// these functions if you have nothing to do in them.

// to launch a service, the naming_service must run.


// this stupid example creates a chat service. it's a 1 person chat, it
// means that you can send strings to the service and it sends them back to you
// (and only to you) so you can't see strings from other people :)
// (i warned you! it's really stupid!)

class CChatService : public IService
{
public:

//	FILE *fp;

	void init ()
	{
		// this function is called after all standard service initialization.
		// put here your code that inits your application.

		nlinfo ("init() was called");

//		fp = nlfopen (NLMISC::CFile::findNewFile("stat.csv"), "wt");
	}

	bool update ()
	{
		// this function is called every "loop". you return true if you want
		// to continue or return false if you want to exit the service.
		// the loop is called evenly (by default, at least one time per second).

		nlinfo ("update() was called");

		/*		static uint ii = 0;
		if(ii++ == 15)
		  {
			nlstop;
			}*/

//////debug to test log flood for memory leak

//		DebugLog->addNegativeFilter("flood");
//		InfoLog->addNegativeFilter("flood");
//		WarningLog->addNegativeFilter("flood");

	  /*		string s;

		uint v = (uint)frand(80), i;
		for (i = 0; i < v; i++)
		{
			s+='a'+rand()%26;
		}
	
		static uint32 val = 0;
		for(i = 0; i < 10; i++)
		{
			nldebug ("debg flood %d %s", val++, s.c_str());
			nlinfo ("info flood %d %s", val++, s.c_str());
			nlwarning ("warn flood %d %s", val++, s.c_str());
//			nldebug ("debg flood %10d %s %d %d", val++, bytesToHumanReadable(CSystemInfo::getAllocatedSystemMemory ()).c_str(), UserSpeedLoop, NetSpeedLoop);
//			nlinfo ("info flood %10d %s %d %d", val++, bytesToHumanReadable(CSystemInfo::getAllocatedSystemMemory ()).c_str(), UserSpeedLoop, NetSpeedLoop);
//			nlwarning ("warn flood %10d %s %d %d", val++, bytesToHumanReadable(CSystemInfo::getAllocatedSystemMemory ()).c_str(), UserSpeedLoop, NetSpeedLoop);
		}
	  */
/////////
//		fprintf (fp, "%d;%d;%d\n", CMemUtils::getAllocatedSystemMemory (), UserSpeedLoop, NetSpeedLoop);
//		fflush (fp);
		return true;
	}

	void release ()
	{
		// this function is called before all standard service release code.
		// put here your code that releases your application.

//		fclose (fp);

		nlinfo ("release() was called");
	}
};


// each time the message CHAT is received, this function is called.
// the first param contains parameters of the message. the second one is the
// identifier of who sent this message
static void cbChat (CMessage &msgin, const std::string &serviceName, TServiceId sid)
{
	// get the chat string of the client
	string chat;
	msgin.serial (chat);

	// create the message to send to the other
	CMessage msgout ("CHAT");
	msgout.serial (chat);

	// send it back to the sender
	CUnifiedNetwork::getInstance()->send (sid, msgout);
}


// this array contains all callback functions. it associates the callbackname (messagename),
// with a callback
TUnifiedCallbackItem CallbackArray[] =
{
	{ "CHAT", cbChat }
};


// this macro is the "main". the first param is the class name inherited from IService.
// the second one is the name of the service used to register and find the service
// using the naming service. the third one is the port where the listen socket will
// be created. If you put 0, the system automatically finds a port.
NLNET_SERVICE_MAIN (CChatService, "CS", "chat_service", 0, CallbackArray, "", "");
