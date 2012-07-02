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


//
// Includes
//

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/config_file.h"

#include "nel/net/udp_sock.h"
#include "nel/net/callback_client.h"
#include "nel/net/inet_address.h"
#include "nel/net/udp_sim_sock.h"

#include "graph.h"


#ifdef USE_3D

#include "nel/3d/u_driver.h"
#include "nel/3d/u_scene.h"
#include "nel/3d/u_camera.h"
#include "nel/3d/u_instance.h"
#include "nel/3d/u_animation_set.h"
#include "nel/3d/u_play_list.h"
#include "nel/3d/u_play_list_manager.h"
#include "nel/3d/u_text_context.h"
#include "nel/3d/u_texture.h"
#include "nel/3d/event_mouse_listener.h"

using namespace NL3D;
#endif

#ifndef UDP_DIR
#define UDP_DIR "."
#endif // UDP_DIR

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;

//
// Variables
//

// must be increase at each version and must be the same value as the server
uint32		Version = 2;

string		ServerAddr = "itsalive.nevrax.org";	// ldserver
uint16		UDPPort = 45455;
uint16		TCPPort = 45456;

uint32		MaxUDPPacketSize = 1000;

CUdpSimSock	*UdpSock = NULL;

uint8		Mode = 0;

uint64		Session = 0;

string		ConnectionName;

CConfigFile	ConfigFile;

#ifdef USE_3D

CGraph FpsGraph ("frame rate (fps)", 10.0f, 110.0f, 100.0f, 100.0f, CRGBA(128,0,0,128), 1000, 150.0f);
CGraph DownloadGraph ("download (bps)", 10.0f, 260.0f, 100.0f, 100.0f, CRGBA(0,0,128,128), 1000, 20000.0f);
CGraph UploadGraph ("upload (bps)", 10.0f, 360.0f, 100.0f, 100.0f, CRGBA(0,128,128,128), 1000, 20000.0f);
CGraph LagGraph ("lag (ms)", 150.0f, 110.0f, 100.0f, 100.0f, CRGBA(128,64,64,128), 100000, 2000.0f);

#endif

//
// Functions
//

void exit (const string &reason)
{
	if (!reason.empty())
		InfoLog->displayRawNL ("%s", reason.c_str());
	InfoLog->displayRawNL ("Press <enter> to exit");
	getchar ();
	exit(EXIT_FAILURE);
}

//
// Config file functions
//

void createConfigFile()
{
	FILE *fp = fopen ("client.cfg", "wt");
	if (fp == NULL)
	{
		InfoLog->displayRawNL ("Can't create client.cfg");
	}
	else
	{
		fprintf (fp, "ServerAddress = \"%s\";\n", ServerAddr.c_str());
		fprintf (fp, "SimInLag = 0;\n");
		fprintf (fp, "SimInPacketLost = 0;\n");
		fprintf (fp, "SimOutLag = 0;\n");
		fprintf (fp, "SimOutPacketLost = 0;\n");
		fprintf (fp, "SimOutPacketDuplication = 0;\n");
		fprintf (fp, "SimOutPacketDisordering = 0;\n");
		fprintf (fp, "ConnectionName = \"\";\n");
		fclose (fp);
	}
}

void checkConnectionName ()
{
	if (ConnectionName.size() > 30)
	{
		exit ("Bad connection name (must be <= 30 characters)");
	}

	if (ConnectionName.size() > 0 && ConnectionName[ConnectionName.size()-1] == '\n')
	{
		ConnectionName = ConnectionName.substr (0, ConnectionName.size()-1);
	}
	
	if (ConnectionName.size() <= 0)
	{
		exit ("Bad connection name (must be > 0 character)");
	}

	for (uint i = 0; i < ConnectionName.size(); i++)
	{
		if (!isalnum(ConnectionName[i]))
		{
			exit ("Bad connection name, only alpha numeric characters is allowed (char '%c' is not alphanum)");
		}
	}
}

void loadConfigFile ()
{
	FILE *fp = fopen ("client.cfg", "rt");
	if (fp == NULL)
	{
		createConfigFile();
	}
	else
	{
		fclose (fp);
	}

	ConfigFile.load ("client.cfg");

	// set internet simulation values
	CUdpSimSock::setSimValues (ConfigFile);

	ServerAddr = ConfigFile.getVar("ServerAddress").asString();

	ConnectionName = ConfigFile.getVar("ConnectionName").asString();

	if (ConnectionName.empty())
	{
		InfoLog->displayRawNL ("Please, enter a connection name");
		InfoLog->displayRawNL ("(only alphanumeric character limited to 30 character, no space)");
		InfoLog->displayRawNL ("For example enter your name and/or your location (ie: \"AceHome\"),");
		InfoLog->displayRawNL ("It'll be use to find your stat file easier:");
		char cn[128];
		if (fgets (cn, 127, stdin) == NULL)
		{
			exit ("Error during the keyboard scanning");
		}
		ConnectionName = cn;
		checkConnectionName ();
		ConfigFile.getVar ("ConnectionName").setAsString(ConnectionName);
		ConfigFile.save ();
	}
	else
	{
		checkConnectionName ();
	}
}


//
// Callbacks
//

void cbInfo (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	string line;
	msgin.serial (line);
	InfoLog->displayRawNL ("%s", line.c_str());

#ifdef USE_3D
	string token = "MeanPongTime ";
	string::size_type pos=line.find (token);
	string::size_type pos2=line.find (" ", pos+token.size());
	float val;
	NLMISC::fromString(line.substr (pos+token.size(), pos2-pos-token.size()), val);	
	LagGraph.addOneValue (val);
#endif
}

void cbInit (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	msgin.serial (Session);

	// create the UDP connection
	nlassert (UdpSock == NULL);
	UdpSock = new CUdpSimSock( false );
	try
	{
		UdpSock->connect( CInetAddress (ServerAddr, UDPPort) );
	}
	catch (const Exception &e)
	{
		InfoLog->displayRawNL ("Cannot connect to remote UDP host '%s': %s", ServerAddr.c_str(), e.what() );
		exit ("");
	}
}

void cbStart (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	InfoLog->displayRawNL ("Bench is starting..");

	Mode = 1;
}

void cbDisconnect (TSockId from, void *arg)
{
	exit ("Lost the server connection. You should not have the last client version\nGet it here: http://www.nevrax.org/download/bench.zip");
}

TCallbackItem CallbackArray[] =
{
	{ "INIT", cbInit },
	{ "INFO", cbInfo },
	{ "START", cbStart },
};



//
// Main
//
int main( int argc, char **argv )
{
	createDebug ();
	DebugLog->addNegativeFilter(" ");
	
	InfoLog->displayRawNL ("\nNevrax UDP benchmark client\n\nPress <CTRL-C> to exit");

	CPath::addSearchPath(UDP_DIR);

	loadConfigFile ();

	CCallbackClient *cc = new CCallbackClient;
	
	cc->addCallbackArray (CallbackArray, sizeof(CallbackArray)/sizeof(CallbackArray[0]));
	cc->setDisconnectionCallback (cbDisconnect, NULL);

	try
	{
		InfoLog->displayRawNL ("Try to connect to %s:%d", ServerAddr.c_str(), TCPPort);
		cc->connect(CInetAddress (ServerAddr, TCPPort));

		CMessage msgout ("INIT");
		msgout.serial (ConnectionName);
		msgout.serial (Version);
		cc->send (msgout);

		InfoLog->displayRawNL ("Waiting the server answer...");
	}
	catch(const Exception &e)
	{
		InfoLog->displayRawNL ("Can't connect to %s:%d (%s)\n", ServerAddr.c_str(), TCPPort, e.what());
		exit ("");
	}

	uint8 *packet = new uint8[MaxUDPPacketSize];
	uint32 psize;

#ifdef USE_3D

	UDriver *Driver = UDriver::createDriver();
	Driver->setDisplay(UDriver::CMode(800, 600, 32, true));
	UScene *Scene= Driver->createScene(false);
	UCamera Camera= Scene->getCam();
	Camera.setTransformMode(UTransform::DirectMatrix);
	UTextContext *TextContext= Driver->createTextContext(CPath::lookup("n019003l.pfb"));
	TextContext->setFontSize(18);

	Camera.setPerspective(80*(float)Pi/180, 1.33f, 0.15f, 1000);

	CEvent3dMouseListener MouseListener;
	MouseListener.addToServer(Driver->EventServer);
	MouseListener.setFrustrum(Camera.getFrustum());
	MouseListener.setHotSpot(CVector(0,0,0));
	CMatrix		initMat;
	initMat.setPos(CVector(0,-5,0));
	MouseListener.setMatrix(initMat);

#endif



	while (cc->connected ())
	{
#ifdef USE_3D

		// Manip.
		Camera.setMatrix(MouseListener.getViewMatrix());

		Driver->EventServer.pump();
		if(Driver->AsyncListener.isKeyPushed(KeyESCAPE))
			return EXIT_SUCCESS;

		Driver->clearBuffers(CRGBA(255,255,255,0));

		Scene->render();

		CGraph::render (*Driver, *TextContext);

		Driver->swapBuffers();

		FpsGraph.addValue (1);

#endif


		CConfigFile::checkConfigFiles ();

		// update TCP connection
		cc->update ();

		// update UDP connection
		if (UdpSock != NULL)
		{
			if (Mode == 0)
			{
				// init the UDP connection
				CMemStream msgout;
				msgout.serial (Mode);
				msgout.serial (Session);
				uint32 size = msgout.length();
#ifdef USE_3D
				UploadGraph.addValue ((float)size);
#endif
				UdpSock->send (msgout.buffer(), size);
				nldebug ("Sent init udp connection");
				nlSleep (100);
			}

			while (UdpSock->dataAvailable())
			{
				psize = MaxUDPPacketSize;
				UdpSock->receive (packet, psize);
#ifdef USE_3D
				DownloadGraph.addValue ((float)psize);
#endif
				CMemStream msgin( true );
				memcpy (msgin.bufferToFill (psize), packet, psize);

				sint64 t = 0;
				msgin.serial (t);

				uint32 p = 0;
				msgin.serial (p);

				uint32 b = 0;
				msgin.serial (b);
	
				// I received a ping, send a pong

				CMemStream msgout;
				msgout.serial (Mode);
				msgout.serial (t);
				msgout.serial (p);
				msgout.serial (b);
				uint8 dummy=0;
				while (msgout.length() < 200)
					msgout.serial (dummy);

				uint32 size =  msgout.length();
				nlassert (size == 200);

#ifdef USE_3D
				UploadGraph.addValue ((float)size);
#endif
				UdpSock->send (msgout.buffer(), size);
			}
		}

		nlSleep (1);
	}
	
	exit ("");
	return EXIT_SUCCESS;
}
