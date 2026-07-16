// client_drone.cpp : Defines the entry point for the application.
//

#include "simulated_client.h"
#include "nel/misc/variable.h"
#include "nel/net/service.h"
#include "client_cfg.h"
#include <fstream>
#ifdef NL_OS_WINDOWS
#include <windows.h>
#endif

using namespace std;
using namespace NLMISC;

const TTime UpdatePeriod = 100;
const TTime ExportPeriod = 2000;

CVariable<bool>	RequestQuit("drone", "RequestQuit", "Set to true to start quiting", false);

/*
 * How to compile client_drone on Linux:
 * . cd code/ryzom/src_v2/client_drone
 * . ln -s ../client/gateway_fec_transport.cpp
 * . touch stdpch.h
 * . ls -1 *.cpp > Objects.mk
 *   Then edit Objects.mk so that it looks like OBJS=obj1.o obj2.o ...
 *   (because 'make update' only generates gateway_fec_transport.o)
 * . make
 */

class CClientDrone : public NLNET::IService
{
public:

	//
	void init(/* uint nbConnections*/ )
	{
		ClientCfg.init( "client_drone.cfg" );
		
		CSheetId::init(false); // allow working without the sheet (only sheet_id.bin is needed)
		CSimulatedClient::initNetwork();
		Clients.resize( ClientCfg.NbConnections );
		for ( uint i=0; i!=Clients.size(); ++i )
		{
			Clients[i] = new CSimulatedClient( i );
			Clients[i]->start();
		}
	}

	//
	bool update()
	{
		static uint nbQuit = 0;
		if ( nbQuit == Clients.size() )
			return false;

		static TTime tLastExport = 0;
		TTime t0 = CTime::getLocalTime();
		if ( t0 - tLastExport >= ExportPeriod )
		{
			exportStates();
			tLastExport = t0;
		}

		// Check for configuration files update.
		CConfigFile::checkConfigFiles();

		// Update clients
		for ( uint i=0; i!=Clients.size(); ++i )
		{
			CSimulatedClient::TLoginState loginStateBeforeUpdating = Clients[i]->getCurrentLoginState();
			bool posInitialized = Clients[i]->UserEntity.isInitialized();

			if ( RequestQuit )
				Clients[i]->requestQuit();

			if ( loginStateBeforeUpdating < CSimulatedClient::LSQuitting )
			{
				if ( ! Clients[i]->update() )
				{
					++nbQuit;
					nldebug("Now %u have quit so far", nbQuit);
				}
			}
			else
				nldebug("Now updating client %u because quitting", i);

			if ( Clients[i]->getCurrentLoginState() != loginStateBeforeUpdating )
				nlinfo( "Client %s to state %u", Clients[i]->name().c_str(), Clients[i]->getCurrentLoginState() );
			if ( Clients[i]->UserEntity.isInitialized() != posInitialized )
				nlinfo( "Client %s placed at %s", Clients[i]->name().c_str(), Clients[i]->UserEntity.pos().asVector().toString().c_str() );
		}

		TTime t1 = CTime::getLocalTime();
		TTime elapsed = t1 - t0;
		if ( elapsed < UpdatePeriod )
			nlSleep( (uint32)(UpdatePeriod - elapsed) );
		t0 = t1;

		if ( RequestQuit )
			RequestQuit = false;

		return true;
	}

	//
	void release()
	{
		// Request disconnection
		nlinfo("Requesting disconnection...");
		RequestQuit = true;

		// Wait for clean disconnection
		while ( update() )
		{
			nlSleep(100);
		}

		// Release
		nlinfo("Releasing...");
		for ( uint i=0; i!=Clients.size(); ++i )
		{
			delete Clients[i];
		}
	}
	
protected:

	//
	void exportStates()
	{
		ofstream f( "client_drone_states.txt" );
		for ( uint i=0; i!=Clients.size(); ++i )
		{
			f << i << ": " << Clients[i]->getCurrentLoginState() << " (" << Clients[i]->UserEntity.pos().asVector().toString().c_str() << ") " << Clients[i]->_NetworkConnection.getMsPerTick() << " ms" << endl;
		}
	}

	std::vector<CSimulatedClient*>	Clients;
};

//CClientDrone Drone;

NLNET::TUnifiedCallbackItem cbArray[1] = { "nop", NULL};

NLNET_SERVICE_MAIN(CClientDrone, "CDRN", "client_drone", 0, cbArray, "", "");
/*
 *
 */
//int APIENTRY WinMain(HINSTANCE hInstance,
//                     HINSTANCE hPrevInstance,
//                     LPSTR     lpCmdLine,
//                     int       nCmdShow)
//{
//	new CApplicationContext;
//
//	// Init
//	createDebug();
//	CFileDisplayer *ClientLogDisplayer = new CFileDisplayer("client_drone.log", true, "CLIENT_DRONE.LOG");
//	DebugLog->addDisplayer (ClientLogDisplayer);
//	InfoLog->addDisplayer (ClientLogDisplayer);
//	WarningLog->addDisplayer (ClientLogDisplayer);
//	ErrorLog->addDisplayer (ClientLogDisplayer);
//	AssertLog->addDisplayer (ClientLogDisplayer);
//	ClientCfg.init( "client_drone.cfg" );
//	if ( ClientCfg.RequestQuit )
//		return 0;
//
//	// Connect and simulate clients
//	Drone.start( ClientCfg.NbConnections );
//	Drone.run();
//
//	// Release
//	Drone.release();
//
//	return 0;
//}



