// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
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

#include "stdpch.h"
#include "game_share/http_client.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


// ***************************************************************************
bool CHttpClient::connect(std::string server)
{
	try
	{
		// Convert an URL to a host if needed
		if (server.substr(0, 7) == "http://")
			server = server.substr(7);
		server = server.substr(0, server.find( "/" ));

		// Add the default port if no port in the cfg, and check if not already connected
		if(server.find(':') == string::npos)
			server+=":80";
		CInetAddress addr = CInetAddress(server);
		if (_Sock.connected())
		{
			if (addr == _Sock.remoteAddr())
				return true;
			else
				_Sock.disconnect();
		}

		// Actually connect
		_Sock.connect(addr);
		if(!_Sock.connected())
		{
			nlwarning("Can't connect to web server '%s'", server.c_str());
			goto end;
		}
		else
		{
			nldebug("Connected to web server '%s'", server.c_str());
		}
	}
	catch(const Exception &e)
	{
		nlwarning("Can't connect to web server '%s': %s", server.c_str(), e.what());
		goto end;
	}

	return true;

end:

	if(_Sock.connected())
		_Sock.close ();

	return false;
}

// ***************************************************************************
bool CHttpClient::send(const std::string& buffer, bool verbose)
{
	nlassert(_Sock.connected());

	if(verbose)
	{
		nldebug("Sending '%s' to '%s'", buffer.c_str(), _Sock.remoteAddr().asString().c_str());
	}

	uint32 size = (uint32)buffer.size();
	
	if(!buffer.empty())
	{
		if(_Sock.send((uint8 *)buffer.c_str(), size, false) != CSock::Ok)
		{
			nlwarning ("Can't send data to the server");
			return false;
		}
	}
	return true;
}

// ***************************************************************************
bool CHttpClient::sendRequest(const std::string& methodWB, const std::string &url, const std::string &cookieName, const std::string &cookieValue, const std::string& postParams, bool verbose)
{
	// Remove the host from the URL
	string path;
	if (url.substr(0, 7) == "http://")
		path = url.substr(7);
	else
		path = url;
	path = path.substr(path.find( "/" ));

	// Send
	if (cookieName.empty() && postParams.empty())
	{
		return send(methodWB + path + "\r\n", verbose);
	}
	else
	{
		string cookieStr, postStr;
		if (!cookieName.empty())
			cookieStr = "Cookie: " + cookieName + "=" + cookieValue + "\r\n";
		if (!postParams.empty())
			postStr = "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: " + toString(postParams.size()) + "\r\n\r\n" + postParams;
		return send(methodWB + path + " HTTP/1.0\r\n" + cookieStr + postStr + "\r\n", verbose);
	}
}

// ***************************************************************************
bool CHttpClient::sendGet(const string &url, const string& params, bool verbose)
{
	return sendRequest("GET ", url + (params.empty() ? "" : ("?" + params)), string(), string(), string(), verbose);
}

// ***************************************************************************
bool CHttpClient::sendGetWithCookie(const string &url, const string &name, const string &value, const string& params, bool verbose)
{
	return sendRequest("GET ", url + (params.empty() ? "" : ("?" + params)), name, value, string(), verbose);
}

// ***************************************************************************
bool CHttpClient::sendPost(const string &url, const string& params, bool verbose)
{
	return sendRequest("POST ", url, string(), string(), params, verbose);
}

// ***************************************************************************
bool CHttpClient::sendPostWithCookie(const string &url, const string &name, const string &value, const string& params, bool verbose)
{
	return sendRequest("POST ", url, name, value, params, verbose);
}

// ***************************************************************************
bool CHttpClient::receive(string &res, bool verbose)
{
	nlassert(_Sock.connected());

	uint32 size;
	res = "";

	uint8 buf[1024];

	if(verbose) nlinfo("Receiving");

	for(;;)
	{
		size = 1023;

		if (_Sock.receive((uint8*)buf, size, false) == CSock::Ok)
		{
			if(verbose) nlinfo("Received OK %d bytes", size);
			buf[1023] = '\0';
			res += (char*)buf;
			//nlinfo("block received '%s'", buf);
		}
		else
		{
			if(verbose) nlinfo("Received CLOSE %d bytes", size);
			buf[size] = '\0';
			res += (char*)buf;
			//nlwarning ("server connection closed");
			break;
		}
	}
	//nlinfo("all received '%s'", res.c_str());
	return true;
}

// ***************************************************************************
void CHttpClient::disconnect()
{
	//if(_Sock.connected())
	// NB Nico : close all the time, to avoid printing into the log after release
	// in CSock dtor -> causes a crash
	_Sock.close ();
}


// ***************************************************************************
CHttpPostTask::CHttpPostTask(const std::string &host, const std::string &page, const std::string &params)
	: _Host(host)
	, _Page(page)
	, _Params(params)
{
}

// ***************************************************************************
void CHttpPostTask::run(void)
{
	CHttpClient httpClient;
	std::string ret;

	if ( ! httpClient.connect(_Host))
	{
		return;
	}
	
	if ( ! httpClient.sendPost(_Host + _Page, _Params))
	{
		return;
	}
	
	httpClient.receive(ret);
	httpClient.disconnect();
}