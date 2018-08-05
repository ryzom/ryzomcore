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
#include <openssl/err.h>

using namespace std;
using namespace NLMISC;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLGUI
{
	//
	// x509CertList lifetime manager
	//
	class SX509Certificates
	{
	public:
		std::vector<X509 *> CertList;
		std::vector<std::string> FilesList;

		bool isUsingOpenSSLBackend;
		bool isInitialized;

		SX509Certificates():isUsingOpenSSLBackend(false), isInitialized(false)
		{
		}

		~SX509Certificates()
		{
			for (uint i = 0; i < CertList.size(); ++i)
			{
				X509_free(CertList[i]);
			}

			CertList.clear();
		}

		void init(CURL *curl)
		{
			if (isInitialized) return;

			// get information on CURL
			curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

			// get more information on CURL session
			curl_tlssessioninfo *sessionInfo;
			CURLcode res = curl_easy_getinfo(curl, CURLINFO_TLS_SSL_PTR, &sessionInfo);

			// only use OpenSSL callback if not using Windows SSPI and using OpenSSL backend
			if (!res && sessionInfo && sessionInfo->backend == CURLSSLBACKEND_OPENSSL && !(data && data->features & CURL_VERSION_SSPI))
			{
#ifdef NL_OS_WINDOWS
				// load native Windows CA Certs
				addCertificatesFrom("CA");
				addCertificatesFrom("AuthRoot");
				addCertificatesFrom("ROOT");

				// we manually loaded native CA Certs, don't need to use custom certificates
				isUsingOpenSSLBackend = false;
#else
				isUsingOpenSSLBackend = true;
#endif
			}
			else
			{
				// if CURL is using SSPI or SChannel under Windows or DarwinSSL under OS X, we'll use native system CA Certs
				isUsingOpenSSLBackend = false;
			}

			isInitialized = true;
		}

#ifdef NL_OS_WINDOWS
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
						CertList.push_back(x509);
					}
				}
				CertFreeCertificateContext(pContext);
				CertCloseStore(hStore, 0);
			}

			// this is called before debug context is set and log ends up in log.log
			nlinfo("Loaded %d certificates from '%s' certificate store", (int)CertList.size(), root);
		}
#endif

		void addCertificatesFromFile(const std::string &cert)
		{
			if (!isUsingOpenSSLBackend) return;

			if (!isInitialized)
			{
				nlwarning("You MUST call NLGUI::CCurlCertificates::init before adding new certificates");
				return;
			}

			// this file was already loaded
			if (std::find(FilesList.begin(), FilesList.end(), cert) != FilesList.end()) return;

			FilesList.push_back(cert);

			// look for certificate in search paths
			string path = CPath::lookup(cert);
			nlinfo("Cert path '%s'", path.c_str());

			if (path.empty())
			{
				nlwarning("Unable to find %s", cert.c_str());
				return;
			}

			CIFile file;

			// open certificate
			if (!file.open(path))
			{
				nlwarning("Unable to open %s", path.c_str());
				return;
			}

			// load certificate content into memory
			std::vector<uint8> buffer(file.getFileSize());
			file.serialBuffer(&buffer[0], file.getFileSize());

			// get a BIO
			BIO *bio = BIO_new_mem_buf(&buffer[0], file.getFileSize());

			if (bio)
			{
				// use it to read the PEM formatted certificate from memory into an X509
				// structure that SSL can use
				STACK_OF(X509_INFO) *info = PEM_X509_INFO_read_bio(bio, NULL, NULL, NULL);

				if (info)
				{
					// iterate over all entries from the PEM file, add them to the x509_store one by one
					for (sint i = 0; i < sk_X509_INFO_num(info); ++i)
					{
						X509_INFO *itmp = sk_X509_INFO_value(info, i);

						if (itmp && itmp->x509)
						{
							CertList.push_back(X509_dup(itmp->x509));
						}
					}

					// cleanup
					sk_X509_INFO_pop_free(info, X509_INFO_free);
				}
				else
				{
					nlwarning("Unable to read PEM info");
				}

				// decrease reference counts
				BIO_free(bio);
			}
			else
			{
				nlwarning("Unable to allocate BIO buffer for certificates");
			}
		}
	};

	/// this will be initialized on startup and cleared on exit
	static SX509Certificates x509CertListManager;

	// ***************************************************************************
	// static
	void CCurlCertificates::init(CURL *curl)
	{
		x509CertListManager.init(curl);
	}

	// ***************************************************************************
	// static
	void CCurlCertificates::addCertificateFile(const std::string &cert)
	{
		x509CertListManager.addCertificatesFromFile(cert);
	}

	// ***************************************************************************
	// static
	CURLcode CCurlCertificates::sslCtxFunction(CURL *curl, void *sslctx, void *parm)
	{
		CURLcode res = CURLE_OK;

		if (x509CertListManager.CertList.size() > 0)
		{
			SSL_CTX *ctx = (SSL_CTX*)sslctx;
			X509_STORE *x509store = SSL_CTX_get_cert_store(ctx);
			if (x509store)
			{
				char errorBuffer[1024];

				for (uint i = 0, ilen = x509CertListManager.CertList.size(); i < ilen; ++i)
				{
					X509_NAME *subject = X509_get_subject_name(x509CertListManager.CertList[i]);

					std::string name;
					unsigned char *tmp = NULL;

					// construct a multiline string with name
					for (int j = 0, jlen = X509_NAME_entry_count(subject); j < jlen; ++j)
					{
						X509_NAME_ENTRY *e = X509_NAME_get_entry(subject, j);
						ASN1_STRING *d = X509_NAME_ENTRY_get_data(e);

						if (ASN1_STRING_to_UTF8(&tmp, d) > 0)
						{
							name += NLMISC::toString("%s\n", tmp);

							OPENSSL_free(tmp);
						}
					}

					// add our certificate to this store
					if (X509_STORE_add_cert(x509store, x509CertListManager.CertList[i]) == 0)
					{
						uint errCode = ERR_get_error();

						// ignore already in hash table errors
						if (ERR_GET_LIB(errCode) != ERR_LIB_X509 || ERR_GET_REASON(errCode) != X509_R_CERT_ALREADY_IN_HASH_TABLE)
						{
							ERR_error_string_n(errCode, errorBuffer, 1024);
							nlwarning("Error adding certificate %s: %s", name.c_str(), errorBuffer);
							res = CURLE_SSL_CACERT;
						}
					}
					else
					{
						nldebug("Added certificate %s", name.c_str());
					}
				}
			}
			else
			{
				nlwarning("SSL_CTX_get_cert_store returned NULL");
			}
		}
		else
		{
			res = CURLE_SSL_CACERT;
		}

		return res;
	}

}// namespace

