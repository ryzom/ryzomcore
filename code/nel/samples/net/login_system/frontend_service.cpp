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
 * Layer 4 and Service example, front-end server.
 *
 * To run this program, ensure there is a file "frontend_service.cfg"
 * containing the location of the naming service (NSHost, NSPort)
 * in the working directory.
 * The NeL naming_service, time_service, login_service, welcome_service must be running.
 */


// We're using the NeL Service framework and layer 5.
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"

#include "nel/net/callback_server.h"
#include "nel/net/service.h"
#include "nel/net/login_server.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLNET;
using namespace NLMISC;

#ifndef NL_LS_CFG
#define NL_LS_CFG ""
#endif // NL_LS_CFG

//#define USE_UDP
#undef USE_UDP


#ifndef USE_UDP

//
// TCP server mode
//

/*
 * Connection callback for a client
 */
void onConnectionClient (TSockId from, const CLoginCookie &cookie)
{
	nlinfo ("The client with uniq Id %d is connected", cookie.getUserId ());

	// store the user id in appId
	from->setAppId (cookie.getUserId ());
}


/*
 * Disconnection callback for a client
 */
void onDisconnectClient (TSockId from, void *arg)
{
	nlinfo ("A client with uniq Id %d has disconnected", from->appId ());

	// tell the login system that this client is disconnected
	CLoginServer::clientDisconnected ((uint32) from->appId ());
}

/*
 * CFrontEndService, based on IService
 */
class CFrontEndService : public IService
{
private:
	/// The server on which the clients connect
	CCallbackServer		_FServer;

public:

	/*
	 * Initialization
	 */
	void init()
	{
		// connect the front end login system
		uint16	fsPort = 37373;
		try
		{
			fsPort = IService::ConfigFile.getVar("FSPort").asInt();
		}
		catch (const EUnknownVar&)
		{
		}
		_FServer.init(fsPort);
		CLoginServer::init (_FServer, onConnectionClient); 

		//
		_FServer.setDisconnectionCallback(onDisconnectClient, NULL);
	}

	bool	update()
	{
		_FServer.update();
		return true;
	}
};

#else // USE_UDP

//
// UDP server mode
//

#include "nel/net/udp_sock.h"

/*
 * CFrontEndService, based on IService
 */
class CFrontEndService : public IService
{
private:
	/// The server on which the clients connect
	CUdpSock *_FServer;

public:

	/*
	 * Initialization
	 */
	void init()
	{
		// connect the front end login system
		uint16	fesPort = 37373;
		try
		{
			fesPort = IService5::ConfigFile.getVar("FESPort").asInt();
		}
		catch (const EUnknownVar&)
		{
		}

		// Socket
		_FServer = new CUdpSock( false );
		nlassert( _FServer );

		// Test of multihomed host
		vector<CInetAddress> addrlist;
		addrlist = CInetAddress::localAddresses();
		vector<CInetAddress>::iterator ivi;
		for ( ivi=addrlist.begin(); ivi!=addrlist.end(); ++ivi )
		{
			nlinfo( "%s", (*ivi).asIPString().c_str() );
		}
		addrlist[0].setPort( fesPort );
		_FServer->bind( addrlist[0] );
		
		CLoginServer::init (*_FServer, NULL); 
	}

	bool	update()
	{
		uint8 buf[64000];
		uint len;
		CInetAddress addr;

		while (_FServer->dataAvailable ())
		{
			len = 64000;
			_FServer->receivedFrom (buf, len, addr);

			CBitMemStream msgin (true);
			msgin.clear ();
			memcpy (msgin.bufferToFill (len), &buf[0], len);

			CLoginCookie lc;
			msgin.serial (lc);

			nlinfo ("Receive the cookie %s from %s", lc.toString ().c_str(), addr.asString ().c_str());

			string res = CLoginServer::isValidCookie (lc);

			// send the result
			CBitMemStream msgout;
			msgout.serial (res);
			uint32 l = msgout.length ();
			_FServer->sendTo (msgout.buffer (), l, addr);
		}
		return true;
	}
};

#endif // USE_UDP

/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is dynamically find and there's no callback array.
 */
NLNET_SERVICE_MAIN (CFrontEndService, "FS", "frontend_service", 0, EmptyCallbackArray, NL_LS_CFG, "")
