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

// We're using std
#include <string>

// We're using NeL
#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/config_file.h"
#include "nel/misc/bit_mem_stream.h"

// We're using the login client
#include "nel/net/login_client.h"
#include "nel/net/login_cookie.h"

#include "nel/net/udp_sock.h"


using namespace std;
using namespace NLMISC;
using namespace NLNET;

/*
 * main
 */
void main (int argc, char **argv)
{
	string result;

	CConfigFile ConfigFile;
	
	ConfigFile.load ("client.cfg");

	string LSHost(ConfigFile.getVar("LSHost").asString());

	char	buf[256];
	printf("Login: ");
	fgets(buf, 256, stdin);
	string	Login(buf);

	printf("Password: ");
	fgets(buf, 256, stdin);
	string	Password(buf);

	if (Login.empty ())
	{
		Login = ConfigFile.getVar("Login").asString();
	}

	if (Password.empty ())
	{
		Password = ConfigFile.getVar("Password").asString();
	}

	/* Try to connect to the login service and check the login, password and version of the client.
	 * return an empty string if all go well
	 */
	result = CLoginClient::authenticate(LSHost+":49999", Login, Password, "sample");
	
	if(!result.empty()) nlerror ("*** Authenticate failed '%s' ***", result.c_str());

	// CLoginClient::ShardList contains all available shards
	for (uint i = 0; i < CLoginClient::ShardList.size (); i++)
	{
		nlinfo("*** shard %d is: %s (%d) ***", i, CLoginClient::ShardList[i].Name.c_str (), CLoginClient::ShardList[i].Id);
	}

	/* Try to connect to the last shard number in the list.
	 * return an empty string if all go well
	 */

	string fs_ip, login_cookie;
	result = CLoginClient::wantToConnectToShard(CLoginClient::ShardList[CLoginClient::ShardList.size() - 1].Id, fs_ip, login_cookie);
	if (!result.empty()) nlerror("*** Select shard failed '%s' ***", result.c_str());
	CLoginCookie cookie; cookie.setFromString(login_cookie); // who's idea was it to send the cookie as a string...
	
	CCallbackClient *cnx = new CCallbackClient();
	result = CLoginClient::connectToShard(cookie, fs_ip, *cnx);

	if (!result.empty()) nlerror("*** Connection to the shard failed '%s' ***", result.c_str());

	nlinfo ("*** Connection granted! You are connected on the frond end ***");

	while (cnx->connected ())
	{
		cnx->update ();
		nlSleep(10);
	}

	if( cnx->connected ())
		cnx->disconnect ();

	delete cnx;
}
