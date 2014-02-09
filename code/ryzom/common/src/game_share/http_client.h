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

#ifndef NL_HTTP_CLIENT_H
#define NL_HTTP_CLIENT_H

#include "nel/misc/types_nl.h"
#include "nel/net/tcp_sock.h"
#include <string>


/**
 * Simple HTTP client
 */
class CHttpClient
{
public:

	/// Constructor
	CHttpClient() {}

	/// Connect to an http server (string by val is intended). If you specify a whole URL, an attempt will be made to determine the server.
	bool connect(std::string server);

	/// Send a 'get' request
	bool sendGet(const std::string &url, const std::string& params=std::string(), bool verbose=false);

	/// Send a 'get' request with a cookie
	bool sendGetWithCookie(const std::string &url, const std::string &name, const std::string &value, const std::string& params=std::string(), bool verbose=false);

	/// Send a 'post' request
	bool sendPost(const std::string &url, const std::string& params=std::string(), bool verbose=false);

	/// Send a 'post' request with a cookie
	bool sendPostWithCookie(const std::string &url, const std::string &name, const std::string &value, const std::string& params=std::string(), bool verbose=false);

	/// Wait for a response
	bool receive(std::string &res, bool verbose=false);

	/// Disconnect if connected (otherwise does nothing)
	void disconnect();

protected:

	/// Helper
	bool sendRequest(const std::string& methodWB, const std::string &url, const std::string &cookieName, const std::string &cookieValue, const std::string& postParams, bool verbose);

	/// Helper
	bool send(const std::string& buffer, bool verbose);

private:

	NLNET::CTcpSock _Sock;
};

// ***************************************************************************
// ***************************************************************************
// ***************************************************************************

class CHttpPostTask : public NLMISC::IRunnable
{
public:
	CHttpPostTask(const std::string &host, const std::string &page, const std::string &params);
	void run(void);

private:
	std::string _Host;
	std::string _Page;
	std::string _Params;
};

#endif // NL_HTTP_CLIENT_H

/* End of http_client.h */
