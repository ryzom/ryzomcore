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

#ifndef CL_CURL_CERTIFICATES_HTML_H
#define CL_CURL_CERTIFICATES_HTML_H

#include "nel/misc/types_nl.h"

#include <curl/curl.h>

namespace NLGUI
{
	class CCurlCertificates
	{
	public:
		// check if compiled with OpenSSL backend
		static void init(CURL *curl);

		// allow to use custom PEM certificates
		static void addCertificateFile(const std::string &cert);

		// cURL SSL certificate loading
		static CURLcode sslCtxFunction(CURL *curl, void *sslctx, void *parm);
	};

} // namespace
#endif
