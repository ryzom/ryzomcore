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

#include "stdnet.h"

#include "nel/misc/system_info.h"

#include "nel/net/callback_client.h"

#include "nel/net/login_cookie.h"
#include "nel/net/login_client.h"

#include "nel/net/udp_sock.h"

using namespace std;
using namespace NLMISC;

namespace NLNET {


CLoginClient::TShardList CLoginClient::ShardList;
CCallbackClient *CLoginClient::_LSCallbackClient;


//
// CALLBACK FROM THE FS (Front-end Service)
//

// Callback for answer of the request shard
static bool ShardValidate;
static string ShardValidateReason;
static void cbShardValidate (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S14: receive "SV" message from FES
	//

	msgin.serial (ShardValidateReason);
	ShardValidate = true;
}

static TCallbackItem FESCallbackArray[] =
{
	{ "SV", cbShardValidate },
};

//
// CALLBACK FROM THE LS (Login Service)
//

// Callback for answer of the login password.
static bool VerifyLoginPassword;
static string VerifyLoginPasswordReason;
static void cbVerifyLoginPassword (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S04: receive the "VLP" message from LS
	//

	msgin.serial (VerifyLoginPasswordReason);
	if(VerifyLoginPasswordReason.empty())
	{
		uint32 nbshard;
		msgin.serial (nbshard);

		CLoginClient::ShardList.clear ();
		VerifyLoginPasswordReason.clear();

		// get the shard list
		for (uint i = 0; i < nbshard; i++)
		{
			CLoginClient::CShardEntry se;
			msgin.serial (se.Name, se.NbPlayers, se.Id);
			CLoginClient::ShardList.push_back (se);
		}		
	}
	VerifyLoginPassword = true;
}

// Callback for answer of the request shard
static bool ShardChooseShard;
static string ShardChooseShardReason;
static string ShardChooseShardAddr;
static string ShardChooseShardCookie;
static void cbShardChooseShard (CMessage &msgin, TSockId from, CCallbackNetBase &netbase)
{
	//
	// S11: receive "SCS" message from LS
	//

	msgin.serial (ShardChooseShardReason);

	if (ShardChooseShardReason.empty())
	{
		msgin.serial (ShardChooseShardCookie);
		msgin.serial (ShardChooseShardAddr);
	}
	ShardChooseShard = true;
}

static TCallbackItem LSCallbackArray[] =
{
	{ "VLP", cbVerifyLoginPassword },
	{ "SCS", cbShardChooseShard },
};

string CLoginClient::authenticate(const string &loginServiceAddr, const ucstring &login, const string &cpassword, const string &application)
{
	string result = authenticateBegin(loginServiceAddr, login, cpassword, application);
	if (!result.empty()) return result;
	while (CLoginClient::authenticateUpdate(result)) nlSleep(10);
	return result;	
}

string CLoginClient::authenticateBegin(const string &loginServiceAddr, const ucstring &login, const string &cpassword, const string &application)
{
	VerifyLoginPasswordReason.clear();
	VerifyLoginPassword = false;

	// S01: connect to the LS
	try
	{
		if(_LSCallbackClient == 0)
		{
			_LSCallbackClient = new CCallbackClient();
			_LSCallbackClient->addCallbackArray(LSCallbackArray, sizeof(LSCallbackArray) / sizeof(LSCallbackArray[0]));
		}

		string addr = loginServiceAddr;
		if(addr.find(":") == string::npos)
			addr += ":49997";
		if(_LSCallbackClient->connected())
			_LSCallbackClient->disconnect();
		_LSCallbackClient->connect (CInetAddress(addr));
	}
	catch (const ESocket &e)
	{
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		nlwarning("Connection refused to LS (addr:%s): %s", loginServiceAddr.c_str(), e.what());
		return toString("Connection refused to LS (addr:%s): %s", loginServiceAddr.c_str(), e.what());
	}
	
	// S02: create and send the "VLP" message
	CMessage msgout("VLP");
	msgout.serial(const_cast<ucstring&>(login));
	msgout.serial(const_cast<string&>(cpassword));
	msgout.serial(const_cast<string&>(application));
	_LSCallbackClient->send(msgout);

	return "";
}

// returns true if it needs to be called again
// error not empty if something went wrong
bool CLoginClient::authenticateUpdate(string &error)
{
	if (!_LSCallbackClient)
	{
		error = "CLoginClient::authenticateBegin() must be called first";
		nlwarning("CLoginClient::authenticateUpdate(): %s", error.c_str());
		return false;
	}
	if (!_LSCallbackClient->connected())
	{
		error = "Disconnected from LS";
		nlwarning("CLoginClient::authenticateUpdate(): %s", error.c_str());
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return false;
	}
	_LSCallbackClient->update();
	if (VerifyLoginPassword)
	{
		error = VerifyLoginPasswordReason;
		if (!error.empty())
		{
			nlwarning("CLoginClient::authenticateUpdate(): %s", error.c_str());
			_LSCallbackClient->disconnect ();
			delete _LSCallbackClient;
			_LSCallbackClient = 0;
		}
		return false;
	}
	return true; // no news, try again
}

string CLoginClient::connectToShard(CLoginCookie &lc, const std::string &addr, CCallbackClient &cnx)
{
	nlassert (!cnx.connected());

	try
	{
		//
		// S12: connect to the FES and send "SV" message to the FES
		//
		cnx.connect (CInetAddress(addr));
		cnx.addCallbackArray (FESCallbackArray, sizeof(FESCallbackArray)/sizeof(FESCallbackArray[0]));

		// send the cookie
		CMessage msgout2 ("SV");
		msgout2.serial (lc);
		cnx.send (msgout2);

		// wait the answer of the connection
		ShardValidate = false;
		while (cnx.connected() && !ShardValidate)
		{
			cnx.update ();
			nlSleep(10);
		}

		// have we received the answer?
		if (!ShardValidate) return "FES disconnect me";
	}
	catch (const ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::connectToShard (const std::string &addr, CUdpSock &cnx)
{
	nlassert (!cnx.connected());

	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		// If a personal firewall such as ZoneAlarm is installed and permission not granted yet,
		// the connect blocks until the user makes a choice.
		// If the user denies the connection, the exception ESocket is thrown.
		// Other firewalls such as Kerio make the send() fail instead.
		//
		cnx.connect (CInetAddress(addr));
	}
	catch (const ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::connectToShard (const std::string &addr, CUdpSimSock &cnx)
{
	nlassert (!cnx.connected());

	
	try
	{
		//
		// S12: connect to the FES. Note: In UDP mode, it's the user that have to send the cookie to the front end
		//
		// See firewall comment in connectToShard(string,CUdpSock)
		//
		cnx.connect (CInetAddress(addr));
	}
	catch (const ESocket &e)
	{
		return string("FES refused the connection (") + e.what () + ")";
	}

	return ShardValidateReason;
}

string CLoginClient::confirmConnection(sint32 shardId)
{
	nlassert(_LSCallbackClient != 0 && _LSCallbackClient->connected());

	//
	// S05: create and send the "CS" message with the shardid choice to the LS
	//
	
	if (ShardList.empty())
	{
		_LSCallbackClient->disconnect();
		return "No shard available";
	}
	
	CLoginClient::CShardEntry *s = getShard(shardId);
	if (!s)
	{
		_LSCallbackClient->disconnect();
		return "Invalid shard selected";
	}

	// send CS
	CMessage msgout ("CS");
	msgout.serial (s->Id);
	_LSCallbackClient->send (msgout);

	// wait the answer
	ShardChooseShard = false;
	while (_LSCallbackClient->connected() && !ShardChooseShard)
	{
		_LSCallbackClient->update ();
		nlSleep(10);
	}

	// have we received the answer?
	if (!ShardChooseShard)
	{
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return "CLoginClientMtp::confirmConnection(): LS disconnects me";
	}
	else
	{
		_LSCallbackClient->disconnect ();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
	}

	if (!ShardChooseShardReason.empty())
	{
		return ShardChooseShardReason;
	}

	// ok, we can try to connect to the good front end

	nlinfo("addr: '%s' cookie: %s", ShardChooseShardAddr.c_str(), ShardChooseShardCookie.c_str());

	return "";
}

string CLoginClient::wantToConnectToShard(sint32 shardId, string &ip, string &cookie)
{
	string res = confirmConnection(shardId);
	if (!res.empty()) return res;

	ip = ShardChooseShardAddr;
	cookie = ShardChooseShardCookie;

	return "";
}

string CLoginClient::selectShardBegin(sint32 shardId)
{
	nlassert(_LSCallbackClient != 0 && _LSCallbackClient->connected());
	
	ShardChooseShardReason.clear();
	ShardChooseShard = false;

	if (ShardList.empty())
	{
		_LSCallbackClient->disconnect();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return "No shard available";
	}	
	CLoginClient::CShardEntry *s = getShard(shardId);
	if (!s)
	{
		_LSCallbackClient->disconnect();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return "Invalid shard selected";
	}

	// S05: create and send the "CS" message with the shardid choice to the LS
	CMessage msgout ("CS");
	msgout.serial (s->Id);
	_LSCallbackClient->send (msgout);

	return "";
}

bool CLoginClient::selectShardUpdate(string &error, string &ip, string &cookie)
{
	if (!_LSCallbackClient)
	{
		error = "CLoginClient::selectShardBegin() must be called first";
		nlwarning("CLoginClient::selectShardUpdate(): %s", error.c_str());
		return false;
	}
	if (!_LSCallbackClient->connected())
	{
		error = "Disconnected from LS";
		nlwarning("CLoginClient::selectShardUpdate(): %s", error.c_str());
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return false;
	}
	_LSCallbackClient->update();
	if (ShardChooseShard)
	{
		error = ShardChooseShardReason;
		ip = ShardChooseShardAddr;
		cookie = ShardChooseShardCookie;
		if (!error.empty()) nlwarning("CLoginClient::selectShardUpdate(): %s", error.c_str());
		else nlinfo("addr: '%s' cookie: %s", ShardChooseShardAddr.c_str(), ShardChooseShardCookie.c_str());
		_LSCallbackClient->disconnect ();
		delete _LSCallbackClient;
		_LSCallbackClient = 0;
		return false;
	}
	return true;
}

CLoginClient::CShardEntry *CLoginClient::getShard (sint32 shardId)
{
	for(TShardList::iterator it=ShardList.begin();it!=ShardList.end();it++)
	{
		if((*it).Id == shardId)
			return &(*it);
	}
	return 0;
}


} // NLNET
