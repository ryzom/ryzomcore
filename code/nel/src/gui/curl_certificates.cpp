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

//#include <crtdbg.h>

#include "stdpch.h"
#include "nel/gui/curl_certificates.h"

#include <openssl/x509.h>
#include <openssl/ssl.h>

#if defined(NL_OS_WINDOWS)
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "cryptui.lib")
#endif

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
#if defined(NL_OS_WINDOWS)
	static std::vector<X509 *> x509CertList;

	//
	// x509CertList lifetime manager
	//
	class SX509Certificates {
	public:
		SX509Certificates()
		{
			curl_version_info_data *data;
			data = curl_version_info(CURLVERSION_NOW);
			if (!(data && data->features & CURL_VERSION_SSPI))
			{
				addCertificatesFrom("CA");
				addCertificatesFrom("AuthRoot");
				addCertificatesFrom("ROOT");
			}
		}

		~SX509Certificates()
		{
			for (uint i = 0; i < x509CertList.size(); ++i)
			{
				X509_free(x509CertList[i]);
			}

			x509CertList.clear();
		}

		void addCertificatesFrom(LPCSTR root)
		{
			HCERTSTORE hStore;
			PCCERT_CONTEXT pContext = NULL;
			X509 *x509;
			hStore = CertOpenSystemStore(NULL, root);
			if (hStore)
			{
				while (pContext = CertEnumCertificatesInStore(hStore, pContext))
				{
					x509 = NULL;
					x509 = d2i_X509(NULL, (const unsigned char **)&pContext->pbCertEncoded, pContext->cbCertEncoded);
					if (x509)
					{
						x509CertList.push_back(x509);
					}
				}
				CertFreeCertificateContext(pContext);
				CertCloseStore(hStore, 0);
			}

			// this is called before debug context is set and log ends up in log.log
			//nlinfo("Loaded %d certificates from '%s' certificate store", List.size(), root);
		}
	};

	/// this will be initialized on startup and cleared on exit
	static SX509Certificates x509CertListManager;

	// ***************************************************************************
	// static
	CURLcode CCurlCertificates::sslCtxFunction(CURL *curl, void *sslctx, void *parm)
	{
		if (x509CertList.size() > 0)
		{
			SSL_CTX *ctx = (SSL_CTX*)sslctx;
			X509_STORE *x509store = SSL_CTX_get_cert_store(ctx);
			if (x509store)
			{
				for (uint i = 0; i < x509CertList.size(); ++i)
				{
					X509_STORE_add_cert(x509store, x509CertList[i]);
				}
			}
			else
			{
				nlwarning("SSL_CTX_get_cert_store returned NULL");
			}
		}
		return CURLE_OK;
	}
#endif // NL_OS_WINDOWS

}// namespace

