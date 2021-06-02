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
 * multi shard example, front-end server.
 *
 * The goal of the example is to enable you to run 2 differents shards
 * on the same computer with a login system.
 *
 * The naming service, admin executor service, login service, welcome service must be running.
 */


// We're using the NeL Service framework and layer 5.
#include "nel/misc/config_file.h"

#include "nel/net/callback_server.h"
#include "nel/net/service.h"
#include "nel/net/login_server.h"

using namespace std;
using namespace NLNET;
using namespace NLMISC;

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
 * Disonnection callback for a client
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
	CCallbackServer		_FEServer;

public:

	/*
	 * Initialization
	 */
	void init()
	{
		// create a special connection for client (using TCP on port 37373)
		uint16	fesPort = 37373;
		try
		{
			fesPort = IService::ConfigFile.getVar("FESPort").asInt();
		}
		catch ( EUnknownVar& )
		{
		}
		_FEServer.init(fesPort);
		
		// connect and identify this front end to the login system
		CLoginServer::init (_FEServer, onConnectionClient); 

		//
		_FEServer.setDisconnectionCallback(onDisconnectClient, NULL);
	}

	bool	update()
	{
		_FEServer.update();
		return true;
	}
};

/*
 * Declare a service with the class CFrontEndService, the names "FS" (short) and "frontend_service" (long).
 * The port is dynamicaly find and there s no callback array.
 */
NLNET_SERVICE_MAIN (CFrontEndService, "FS", "frontend_service", 0, EmptyCallbackArray, "", "")
