// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_CURL_HTTP_CLIENT_H
#define NL_CURL_HTTP_CLIENT_H

#include "nel/misc/types_nl.h"
#include <string>

namespace NLWEB
{

/**
	* HTTP client with SSL capabilities
	*/
class CCurlHttpClient
{
public:

	/// Constructor
	CCurlHttpClient() : _CurlStruct(NULL) {}

	/// Connect to an http server (string by val is intended). If you specify a whole URL, an attempt will be made to determine the server.
	bool connect(const std::string &server);

	/// Authenticate with "user:password" (only for restricted websites)
	bool authenticate(const std::string &user, const std::string &password);

	/// For a https server, call this with true to check for server certificate and host
	bool verifyServer(bool verify);

	/// Send a 'get' request
	bool sendGet(const std::string &url, const std::string &params = std::string(), bool verbose = false);

	/// Send a 'get' request with a cookie
	bool sendGetWithCookie(const std::string &url, const std::string &name, const std::string &value, const std::string &params = std::string(), bool verbose = false);

	/// Send a 'post' request
	bool sendPost(const std::string &url, const std::string &params = std::string(), bool verbose = false);

	/// Send a 'post' request with a cookie
	bool sendPostWithCookie(const std::string &url, const std::string &name, const std::string &value, const std::string &params = std::string(), bool verbose = false);

	/// Wait for a response
	bool receive(std::string &res, bool verbose = false);

	/// Disconnect if connected (otherwise does nothing)
	void disconnect();

protected:

	/// Helper
	bool sendRequest(const std::string &methodWB, const std::string &url, const std::string &cookieName, const std::string &cookieValue, const std::string &postParams, bool verbose);

	/// Helper
	void pushReceivedData(uint8 *buffer, uint size);

	static size_t writeDataFromCurl(void *buffer, size_t size, size_t nmemb, void *pHttpClient);
private:

	void *_CurlStruct; // void* to prevent including curl.h in a header file

	std::vector<uint8>	_ReceiveBuffer;
	std::string			_Auth; // must be kept here because curl only stores the char pointer
};

extern CCurlHttpClient CurlHttpClient;

}

#endif // NL_HTTP_CLIENT_H

/* End of http_client_curl.h */
