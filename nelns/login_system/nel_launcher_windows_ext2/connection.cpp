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

//
// Includes
//

#include "std_afx.h"

#include <nel/misc/md5.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/thread.h>
#include <nel/net/tcp_sock.h>
#include <nel/net/login_client.h>

#include "nel_launcher_dlg.h"
#include "connection.h"


//
// Namespaces
//

using namespace std;
using namespace NLMISC;
using namespace NLNET;


//
// Variables
//

vector<CShard> Shards;

static CTcpSock sock;

static string Login, Password, ClientApp;

//
// Functions
//

static bool connect()
{
	string server = ConfigFile.getVar("StartupHost").asString(0);

	if(sock.connected())
		return true;

	try
	{
		// add the default port if no port in the cfg
		if(server.find(':') == string::npos)
			server+=":80";
		sock.connect(CInetAddress(server));
		if(!sock.connected())
		{
			nlwarning("Can't connect to web server '%s'", server.c_str());
			goto end;
		}
	}
	catch(Exception &e)
	{
		nlwarning("Can't connect to web server '%s': %s", server.c_str(), e.what());
		goto end;
	}
	
	return true;

end:

	if(sock.connected())
		sock.close ();
	
	return false;
}

static bool send(const string &url)
{
	nlassert(sock.connected());

	string buffer = "GET " + url + "\r\n";
	uint32 size = buffer.size();
	if(!url.empty())
	{
		if(sock.send((uint8 *)buffer.c_str(), size, false) != CSock::Ok)
		{
			nlwarning ("Can't send data to the server");
			return false;
		}
	}
	return true;
}

static bool receive(string &res)
{
	nlassert(sock.connected());

	uint32 size;
	res = "";

	uint8 buf[1024];

	if(VerboseLog) nlinfo("Receiving");

	while (true)
	{
		size = 1023;

		if (sock.receive((uint8*)buf, size, false) == CSock::Ok)
		{
			if(VerboseLog) nlinfo("Received OK %d bytes", size);
			buf[1023] = '\0';
			res += (char*)buf;
			//nlinfo("block received '%s'", buf);
		}
		else
		{
			if(VerboseLog) nlinfo("Received CLOSE %d bytes", size);
			buf[size] = '\0';
			res += (char*)buf;
			//nlwarning ("server connection closed");
			break;
		}
	}
	//nlinfo("all received '%s'", res.c_str());
	return true;
}

string checkLogin(const string &login, const string &password, const string &clientApp)
{
	Shards.clear();
	Login = Password = ClientApp = "";

	if (ConfigFile.getVar("UseDirectClient").asBool())
	{
		ucstring pwd = ucstring(password);
		CHashKeyMD5 hk = getMD5((uint8*)pwd.c_str(), pwd.size());
		string cpwd = hk.toString();
		nlinfo("The crypted password is %s", cpwd.c_str());
		string result = CLoginClient::authenticate(ConfigFile.getVar("StartupHost").asString(), login, cpwd, clientApp);
		if (!result.empty()) return result;
		for(uint i = 0; i < CLoginClient::ShardList.size(); ++i)
		{
			nldebug("Shard '%u' '%s' '%u'", CLoginClient::ShardList[i].Id, CLoginClient::ShardList[i].Name.toString().c_str(), CLoginClient::ShardList[i].NbPlayers);
			Shards.push_back(CShard("1", true, 
				CLoginClient::ShardList[i].Id, CLoginClient::ShardList[i].Name.toString(), CLoginClient::ShardList[i].NbPlayers, 
				"1", "1"));
		}
		Login = login;
		Password = password;
		ClientApp = clientApp;
		return "";
	}

	if(!connect())
		return "Can't connect (error code 1)";

	if(VerboseLog) nlinfo("Connected");

	if(!send(ConfigFile.getVar("StartupPage").asString()+"?login="+login+"&password="+password+"&clientApplication="+clientApp))
		return "Can't send (error code 2)";

	if(VerboseLog) nlinfo("Sent request login check");

	string res;

	if(!receive(res))
		return "Can't receive (error code 3)";

	if(VerboseLog) nlinfo("Received request login check");

	if(res.empty())
		return "Empty answer from server (error code 4)";

	if(res[0] == '0')
	{
		// server returns an error
		nlwarning("server error: %s", res.substr(2).c_str());
		return res.substr(2);
	}
	else if(res[0] == '1')
	{
		// server returns ok, we have the list of shard
		uint nbs = atoi(res.substr(2).c_str());
		vector<string> lines;
		
		explode(res, string("\n"), lines, true);

		if(VerboseLog)
		{
			nlinfo ("Exploded, with nl, %d res", lines.size());
/*			for (uint i = 0; i < lines.size(); i++)
			{
				nlinfo (" > '%s'", lines[i].c_str());
			}*/
		}

		if(lines.size() != nbs+1)
		{
			nlwarning("bad shard lines number %d != %d", lines.size(), nbs+1);
			nlwarning("'%s'", res.c_str());
			return "bad lines numbers (error code 5)";
		}

		for(uint i = 1; i < lines.size(); i++)
		{
			vector<string> res;
			explode(lines[i], string("|"), res);

			if(VerboseLog)
			{
				nlinfo ("Exploded with '%s', %d res", "|", res.size());
/*				for (uint i = 0; i < res.size(); i++)
				{
					nlinfo (" > '%s'", res[i].c_str());
				}*/
			}

			if(res.size() != 7)
			{
				nlwarning("bad | numbers %d != %d", res.size(), 7);
				nlwarning("'%s'", lines[i].c_str());
				return "bad pipe numbers (error code 6)";
			}
			Shards.push_back(CShard(res[0], atoi(res[1].c_str())>0, atoi(res[2].c_str()), res[3], atoi(res[4].c_str()), res[5], res[6]));
		}
	}
	else
	{
		// server returns ???
		nlwarning("%s", res.c_str());
		return res;
	}

	Login = login;
	Password = password;
	ClientApp = clientApp;

	return "";
}

string selectShard(uint32 shardId, string &cookie, string &addr)
{
	cookie = addr = "";

	if (ConfigFile.getVar("UseDirectClient").asBool())
		return CLoginClient::wantToConnectToShard(shardId, addr, cookie);

	if(!connect()) return "Can't connect (error code 7)";

	if(Login.empty()) return "Empty Login (error code 8)";
	if(Password.empty()) return "Empty Password (error code 9)";
	if(ClientApp.empty()) return "Empty Client Application (error code 10)";
	
	if(!send(ConfigFile.getVar("StartupPage").asString()+"?cmd=login&shardid="+toString(shardId)+"&login="+Login+"&password="+Password+"&clientApplication="+ClientApp))
		return "Can't send (error code 11)";
	
	string res;
	
	if(!receive(res))
		return "Can't receive (error code 12)";

	if(res.empty())
		return "Empty result (error code 13)";

	if(res[0] == '0')
	{
		// server returns an error
		nlwarning("server error: %s", res.substr(2).c_str());
		return res.substr(2);
	}
	else if(res[0] == '1')
	{
		// server returns ok, we have the access
		
		vector<string> line;
		explode(res, string(" "), line, true);

		if(line.size() != 2)
		{
			nlwarning("bad launch lines number %d != %d", line.size(), 2);
			return "bad launch line number (error code 14)";
		}
		
		cookie = line[0].substr(2);
		addr = line[1];
	}
	else
	{
		// server returns ???
		nlwarning("%s", res.c_str());
		return res;
	}

	return "";
}
