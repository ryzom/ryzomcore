// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "stdweb.h"
#include <nel/web/http_client_curl.h>

#include <nel/misc/debug.h>
#include <nel/web/curl_certificates.h>

using namespace std;
using namespace NLMISC;
using namespace NLWEB;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

#define _Curl (CURL *)_CurlStruct

namespace NLWEB
{

// Ugly CURL callback
size_t CCurlHttpClient::writeDataFromCurl(void *buffer, size_t size, size_t nmemb, void *pHttpClient)
{
	CCurlHttpClient * httpClient = static_cast<CCurlHttpClient*>(pHttpClient);
	httpClient->pushReceivedData((uint8*)buffer, (uint)(size*nmemb));
	return size*nmemb;
}

// ***************************************************************************
bool CCurlHttpClient::connect(const std::string &/* server */)
{
	curl_version_info_data *vdata = curl_version_info(CURLVERSION_NOW);
	nldebug("Libcurl v%s", vdata->version);
	if ((vdata->features & CURL_VERSION_SSL) == 0)
		nlwarning("SSL not supported");

	curl_global_init(CURL_GLOBAL_ALL);
	_CurlStruct = curl_easy_init();
	if(_Curl == NULL)
	{
		curl_global_cleanup();
		return false;
	}

	return true;
}

// ***************************************************************************
bool CCurlHttpClient::authenticate(const std::string &user, const std::string &password)
{
	_Auth = user + ":" + password;
	return true;
}

static const std::string CAFilename = "cacert.pem"; // https://curl.haxx.se/docs/caextract.html

// ***************************************************************************
bool CCurlHttpClient::verifyServer(bool verify)
{
	curl_easy_setopt(_Curl, CURLOPT_SSL_VERIFYHOST, verify ? 2 : 0);
	curl_easy_setopt(_Curl, CURLOPT_SSL_VERIFYPEER, verify ? 1 : 0);

	// specify custom CA certs
	CCurlCertificates::addCertificateFile(CAFilename);

	// if supported, use custom SSL context function to load certificates
	CCurlCertificates::useCertificates(_Curl);

	return true;
}

// ***************************************************************************
bool CCurlHttpClient::sendRequest(const std::string& methodWB, const std::string &url, const std::string &cookieName, const std::string &cookieValue, const std::string& postParams, bool verbose)
{
	if (verbose)
		curl_easy_setopt(_Curl, CURLOPT_VERBOSE, 1);

	// Set URL
	curl_easy_setopt(_Curl, CURLOPT_URL, url.c_str());
	if (url.length() > 8 && (url[4] == 's' || url[4] == 'S')) // 01234 https
	{
		curl_easy_setopt(_Curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(_Curl, CURLOPT_SSL_VERIFYHOST, 2L);
	}

	// Authentication
	if (!_Auth.empty())
	{
		curl_easy_setopt(_Curl, CURLOPT_HTTPAUTH, CURLAUTH_BASIC); // TODO: CURLAUTH_ANY
		curl_easy_setopt(_Curl, CURLOPT_USERPWD, _Auth.c_str());
	}

	// Set POST params
	if ((methodWB == "POST") && (!postParams.empty()))
	{
		curl_easy_setopt(_Curl, CURLOPT_POSTFIELDS, postParams.c_str());
	}
	// Cookie
	if (!cookieName.empty())
		curl_easy_setopt(_Curl,	CURLOPT_COOKIE, string(cookieName+"="+cookieValue).c_str());

	// Include the header in the response
	curl_easy_setopt(_Curl, CURLOPT_HEADER, 1);

	// Register the receive callback
	curl_easy_setopt(_Curl, CURLOPT_WRITEFUNCTION, CCurlHttpClient::writeDataFromCurl);
	curl_easy_setopt(_Curl, CURLOPT_WRITEDATA, this);

	char errorbuf [CURL_ERROR_SIZE+1];
	curl_easy_setopt(_Curl, CURLOPT_ERRORBUFFER, errorbuf);

	// Send
	CURLcode res = curl_easy_perform(_Curl);
	if (res != 0)
	{
		if (verbose)
			nlwarning(errorbuf);
		return false;
	}

	// Get result
	long r;
	curl_easy_getinfo(_Curl, CURLINFO_RESPONSE_CODE, &r);
	if (verbose)
	{
		nldebug("%u", (uint)r);
	}

	return true;
}

void CCurlHttpClient::pushReceivedData(uint8 *buffer, uint size)
{
	_ReceiveBuffer.insert(_ReceiveBuffer.end(), buffer, buffer+size);
}

// ***************************************************************************
bool CCurlHttpClient::sendGet(const string &url, const string& params, bool verbose)
{
	return sendRequest("GET", url + (params.empty() ? "" : ("?" + params)), string(), string(), string(), verbose);
}

// ***************************************************************************
bool CCurlHttpClient::sendGetWithCookie(const string &url, const string &name, const string &value, const string& params, bool verbose)
{
	return sendRequest("GET", url + (params.empty() ? "" : ("?" + params)), name, value, string(), verbose);
}

// ***************************************************************************
bool CCurlHttpClient::sendPost(const string &url, const string& params, bool verbose)
{
	return sendRequest("POST", url, string(), string(), params, verbose);
}

// ***************************************************************************
bool CCurlHttpClient::sendPostWithCookie(const string &url, const string &name, const string &value, const string& params, bool verbose)
{
	return sendRequest("POST", url, name, value, params, verbose);
}

// ***************************************************************************
bool CCurlHttpClient::receive(string &res, bool verbose)
{
	if (verbose)
	{
		nldebug("Receiving %u bytes", (uint)_ReceiveBuffer.size());
	}

	res.clear();
	if (!_ReceiveBuffer.empty())
		res.assign((const char*)&(*(_ReceiveBuffer.begin())), _ReceiveBuffer.size());
	_ReceiveBuffer.clear();
	return true;
}

// ***************************************************************************
void CCurlHttpClient::disconnect()
{
	if (_CurlStruct)
	{
		curl_easy_cleanup(_Curl);
		_CurlStruct = NULL;
		curl_global_cleanup();
	}
}

CCurlHttpClient CurlHttpClient;

}

/* end of file */
