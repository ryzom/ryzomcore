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
#include "nel/misc/md5.h"

#include "nel/net/login_client.h"
#include "nel/net/login_cookie.h"

// The UDP mode wasn't tested, you should leave USE_UDP undef
//#define USE_UDP
#undef USE_UDP

#ifdef USE_UDP
#	include "nel/net/udp_sock.h"
#endif

#ifndef NL_LS_CFG
#define NL_LS_CFG "."
#endif // NL_LS_CFG

//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Functions
//

int main (int argc, char **argv)
{
	string result;
	
	CApplicationContext myApplicationContext;

	CPath::addSearchPath(NL_LS_CFG);

	CConfigFile ConfigFile;
	
	ConfigFile.load ("client.cfg");

	string LSHost(ConfigFile.getVar("LSHost").asString());

	ucstring Login = ConfigFile.getVar("Login").asString();
	if(Login.empty())
	{
		char buf[256];
		printf("Login: ");
		Login = fgets(buf, 256, stdin);
	}

	ucstring Password = ConfigFile.getVar("Password").asString();
	if(Password.empty())
	{
		char buf[256];
		printf("Password: ");
		Password = fgets(buf, 256, stdin);
	}
	// crypt with md5 the password
	CHashKeyMD5 hk = getMD5((uint8*)Password.c_str(), (uint32)Password.size());
	string CPassword = hk.toString();
	nlinfo("The crypted password is %s", CPassword.c_str());

	string Application = ConfigFile.getVar("ClientApplication").asString();

	/* Try to connect to the login service and check the login, password of the client.
	 * return an empty string if all go well
	 */
	result = CLoginClient::authenticate(LSHost, Login, CPassword, Application);

	if(!result.empty()) nlerror ("*** Authenticate failed '%s' ***", result.c_str());

	nlinfo("%d Shards are available:", CLoginClient::ShardList.size());
	for (uint i = 0; i < CLoginClient::ShardList.size (); i++)
	{
		nlinfo ("    ShardId %3d: %s (%d online players)", CLoginClient::ShardList[i].Id, CLoginClient::ShardList[i].Name.toUtf8().c_str (), CLoginClient::ShardList[i].NbPlayers);
	}

	sint32 sid = ConfigFile.getVar("ShardId").asInt();
	if(sid == 0)
	{
		printf("Enter the ShardId you want to connect to: ");
		if (scanf("%d", &sid) != 1)
		{
			nlwarning("Can't parse ShardId");
		}
	}

	/* Try to connect to the shard number 0 in the list.
	 * return an empty string if all go well
	 */

	string ip, cookie;
	result = CLoginClient::wantToConnectToShard (sid, ip, cookie);

	if(!result.empty()) nlerror ("*** Connection to the shard failed '%s' ***", result.c_str());

#ifndef USE_UDP

	CCallbackClient *cnx = new CCallbackClient();
	CLoginCookie lc;
	lc.setFromString(cookie);
	result = CLoginClient::connectToShard (lc, ip, *cnx);

	if(!result.empty()) nlerror ("*** Connection to the shard failed '%s' ***", result.c_str());

	nlinfo ("*** Connection granted! You are connected on the frond end ***");

	while (cnx->connected ())
	{
		cnx->update ();
		nlSleep(10);
	}

	if( cnx->connected ())
		cnx->disconnect ();

#else // USE_UDP

	CUdpSock *cnx = new CUdpSock();
	CLoginCookie lc;
	result = CLoginClient::connectToShard (0, *cnx, lc);

	if(!result.empty()) nlerror ("*** Connection to the shard failed '%s' ***", result.c_str());

	// we have to talk with the FES to send the cookie

	CBitMemStream msgout;
	msgout.serial (lc);
	while (true)
	{
		// send the cookie
		uint32 len = msgout.length ();
		cnx->send (msgout.buffer (), len);

		uint8 buf[64000];
		if (cnx->dataAvailable ())
		{
			// manage answer
			len = 64000;
			cnx->receive (buf, len);

			CBitMemStream msgin (true);
			msgin.clear ();
			memcpy (msgin.bufferToFill (len), &buf[0], len);

			msgin.serial (result);
			break;
		}

		// sleep
		nlSleep (1000);
	}

	if(!result.empty()) nlerror ("*** Connection to the shard failed '%s' ***", result.c_str());

	nlinfo ("*** Connection granted! You are connected on the frond end ***");

	while (cnx->connected ())
	{
		// do want you want!

		nlSleep(10);
	}

#endif // USE_UDP

	delete cnx;
	return 0;
}
