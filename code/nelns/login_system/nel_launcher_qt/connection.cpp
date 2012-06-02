#include "connection.h"
#include <iostream>
#include <nel/misc/ucstring.h>
#include <nel/misc/md5.h>
#include <nel/net/sock.h>
#include <nel/net/login_client.h>

#include "nel_launcher_dlg.h"

bool CNelLauncherConnection::connect()
{
	std::string server = ConfigFile.getVar("StartupHost").asString(0);

	if(m_Sock.connected())
		return true;

	try
	{
		// add the default port if no port in the cfg
		if(server.find(':') == std::string::npos)
			server+=":80";
		m_Sock.connect(NLNET::CInetAddress(server));
		if(!m_Sock.connected())
		{
			nlwarning("Can't connect to web server '%s'", server.c_str());
			goto end;
		}
	}
	catch(NLMISC::Exception &e)
	{
		nlwarning("Can't connect to web server '%s': %s", server.c_str(), e.what());
		goto end;
	}

	return true;

end:

	if(m_Sock.connected())
		m_Sock.close ();

	return false;
}

bool CNelLauncherConnection::send(const std::string &url)
{
	nlassert(m_Sock.connected());

	std::string buffer = "GET " + url + "\r\n";
	uint32 size = buffer.size();
	if(!url.empty())
	{
		if(m_Sock.send((uint8 *)buffer.c_str(), size, false) != NLNET::CSock::Ok)
		{
			nlwarning ("Can't send data to the server");
			return false;
		}
	}
	return true;
}

bool CNelLauncherConnection::receive(std::string &res)
{
	nlassert(m_Sock.connected());

	uint32 size;
	res = "";

	uint8 buf[1024];

	//if(VerboseLog) nlinfo("Receiving");

	while (true)
	{
		size = 1023;

		if(m_Sock.receive((uint8*)buf, size, false) == NLNET::CSock::Ok)
		{
			//if(VerboseLog) nlinfo("Received OK %d bytes", size);
			buf[1023] = '\0';
			res += (char*)buf;
			//nlinfo("block received '%s'", buf);
		}
		else
		{
			//if(VerboseLog) nlinfo("Received CLOSE %d bytes", size);
			buf[size] = '\0';
			res += (char*)buf;
			//nlwarning ("server connection closed");
			break;
		}
	}

	// trim off whitespace.
	res = NLMISC::trim(res);
	nlinfo("all received '%s'", res.c_str());
	return true;
}

std::string CNelLauncherConnection::checkLogin(const std::string &login, const std::string &password, const std::string &clientApp)
{
	m_Shards.clear();
	m_Login = m_Password = m_ClientApp = "";

	if(ConfigFile.exists("UseDirectClient") && ConfigFile.getVar("UseDirectClient").asBool())
	{
		ucstring pwd = ucstring(password);
		NLMISC::CHashKeyMD5 hk = NLMISC::getMD5((uint8*)pwd.c_str(), pwd.size());
		std::string cpwd = hk.toString();
		nlinfo("The crypted password is %s", cpwd.c_str());
		std::string result = NLNET::CLoginClient::authenticate(ConfigFile.getVar("StartupHost").asString(), login, cpwd, clientApp);
		if (!result.empty()) return result;
		for(uint i = 0; i < NLNET::CLoginClient::ShardList.size(); ++i)
		{
			nldebug("Shard '%u' '%s' '%u'", NLNET::CLoginClient::ShardList[i].Id, NLNET::CLoginClient::ShardList[i].Name.toString().c_str(), NLNET::CLoginClient::ShardList[i].NbPlayers);
			m_Shards.push_back(CShard("1", true,
				NLNET::CLoginClient::ShardList[i].Id, NLNET::CLoginClient::ShardList[i].Name.toString(), NLNET::CLoginClient::ShardList[i].NbPlayers,
				"1", "1"));
		}
		m_Login = login;
		m_Password = password;
		m_ClientApp = clientApp;
		return "";
	}

	if(!connect())
		return "Can't connect (error code 1)";

	//if(VerboseLog) nlinfo("Connected");

	if(!send(ConfigFile.getVar("StartupPage").asString()+"?login="+login+"&password="+password+"&clientApplication="+clientApp))
		return "Can't send (error code 2)";

	//if(VerboseLog) nlinfo("Sent request login check");

	std::string res;

	if(!receive(res))
		return "Can't receive (error code 3)";

	//if(VerboseLog) nlinfo("Received request login check");

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
		std::vector<std::string> lines;

		NLMISC::explode(res, std::string("\n"), lines, true);

	//	if(VerboseLog)
	//	{
			//nlinfo ("Exploded, with nl, %d res", lines.size());
/*		      for (uint i = 0; i < lines.size(); i++)
			{
				nlinfo (" > '%s'", lines[i].c_str());
			}*/
	//	}

		if(lines.size() != nbs+1)
		{
			nlwarning("bad shard lines number %d != %d", lines.size(), nbs+1);
			nlwarning("'%s'", res.c_str());
			return "bad lines numbers (error code 5)";
		}

		for(uint i = 1; i < lines.size(); i++)
		{
			std::vector<std::string> res;
			NLMISC::explode(lines[i], std::string("|"), res);

	//		if(VerboseLog)
	//		{
	//			nlinfo ("Exploded with '%s', %d res", "|", res.size());
/*			      for (uint i = 0; i < res.size(); i++)
				{
					nlinfo (" > '%s'", res[i].c_str());
				}*/
	//		}

			if(res.size() != 7)
			{
				nlwarning("bad | numbers %d != %d", res.size(), 7);
				nlwarning("'%s'", lines[i].c_str());
				return "bad pipe numbers (error code 6)";
			}
			m_Shards.push_back(CShard(res[0], atoi(res[1].c_str())>0, atoi(res[2].c_str()), res[3], atoi(res[4].c_str()), res[5], res[6]));
		}
	}
	else
	{
		// server returns ???
		nlwarning("%s", res.c_str());
		return res;
	}

	m_Login = login;
	m_Password = password;
	m_ClientApp = clientApp;

	return "";
}

std::string CNelLauncherConnection::selectShard(uint32 shardId, std::string &cookie, std::string &addr)
{
	cookie = addr = "";

	if(ConfigFile.exists("UseDirectClient") && ConfigFile.getVar("UseDirectClient").asBool())
		return NLNET::CLoginClient::wantToConnectToShard(shardId, addr, cookie);

	if(!connect()) return "Can't connect (error code 7)";

	if(m_Login.empty()) return "Empty Login (error code 8)";
	if(m_Password.empty()) return "Empty Password (error code 9)";
	if(m_ClientApp.empty()) return "Empty Client Application (error code 10)";

	if(!send(ConfigFile.getVar("StartupPage").asString()+"?cmd=login&shardid="+NLMISC::toString(shardId)+"&login="+m_Login+"&password="+m_Password+"&clientApplication="+m_ClientApp))
		return "Can't send (error code 11)";

	std::string res;

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

		std::vector<std::string> line;
		NLMISC::explode(res, std::string(" "), line, true);

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

