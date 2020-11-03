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
#include <nel/web/curl_certificates.h>

#include <nel/misc/common.h>
#include <nel/misc/debug.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>

// for compatibility with older versions
#ifndef CURL_AT_LEAST_VERSION
#define CURL_VERSION_BITS(x,y,z) ((x)<<16|(y)<<8|z)
#define CURL_AT_LEAST_VERSION(x,y,z) \
  (LIBCURL_VERSION_NUM >= CURL_VERSION_BITS(x, y, z))
#endif


using namespace std;
using namespace NLMISC;
using namespace NLWEB;

#ifdef DEBUG_NEW
#define new DEBUG_NEW
#endif

namespace NLWEB
{
	//
	// x509CertList lifetime manager
	//
	class SX509Certificates
	{
	public:
		struct CertEntry
		{
			X509 *cert;
			std::string name;
			std::string file;

			bool operator == (const std::string &str)
			{
				return file == str;
			}
		};

		std::vector<CertEntry> CertList;

		bool isUsingOpenSSLBackend;
		bool isInitialized;

		SX509Certificates():isUsingOpenSSLBackend(false), isInitialized(false)
		{
			init();
		}

		~SX509Certificates()
		{
			for (uint i = 0; i < CertList.size(); ++i)
			{
				X509_free(CertList[i].cert);
			}

			CertList.clear();
		}

		void init()
		{
			// init CURL
			CURL *curl = curl_easy_init();
			if (!curl) return;

			// get information on CURL
			curl_version_info_data *data = curl_version_info(CURLVERSION_NOW);

			bool useOpenSSLBackend = false;

#if CURL_AT_LEAST_VERSION(7, 34, 0)
			// get more information on CURL session
			curl_tlssessioninfo *sessionInfo;

			CURLINFO info;

#if CURL_AT_LEAST_VERSION(7, 48, 0)
			info = CURLINFO_TLS_SSL_PTR;
#else
			info = CURLINFO_TLS_SESSION;
#endif

			CURLcode res = curl_easy_getinfo(curl, info, &sessionInfo);

			// CURL using OpenSSL backend
			if ((res == CURLE_OK) && sessionInfo && sessionInfo->backend == CURLSSLBACKEND_OPENSSL) useOpenSSLBackend = true;
#elif CURL_AT_LEAST_VERSION(7, 12, 3)
			// get a list of OpenSSL engines
			struct curl_slist *engines;

			CURLcode res = curl_easy_getinfo(curl, CURLINFO_SSL_ENGINES, &engines);

			// CURL using OpenSSL backend
			// With OpenSSL compiled without any engine, engines will too return NULL
			// Fortunately, if OpenSSL isn't compiled with engines means we compiled it ourself and CURL is a recent version
			if ((res == CURLE_OK) && engines)
			{
				// free engines
				curl_slist_free_all(engines);

				useOpenSSLBackend = true;
			}
#else
			// TODO: implement an equivalent, but CURL 7.12 was released in 2004
#endif

			// only use OpenSSL callback if not using Windows SSPI and using OpenSSL backend
			if (useOpenSSLBackend && !(data && data->features & CURL_VERSION_SSPI))
			{
#ifdef NL_OS_WINDOWS
				// load native Windows CA Certs
				addCertificatesFrom("CA");
				addCertificatesFrom("AuthRoot");
				addCertificatesFrom("ROOT");
#endif

				isUsingOpenSSLBackend = true;
			}
			else
			{
				// if CURL is using SSPI or SChannel under Windows or DarwinSSL under OS X, we'll use native system CA Certs
				isUsingOpenSSLBackend = false;
			}

			// clean up CURL
			curl_easy_cleanup(curl);

			isInitialized = true;
		}

		static std::string getCertName(X509 *cert)
		{
			// NULL certificate
			if (!cert) return "";

			X509_NAME *subject = X509_get_subject_name(cert);

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

			return name;
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
						CertEntry entry;
						entry.cert = x509;
						entry.file = root;
						entry.name = getCertName(x509);

						CertList.push_back(entry);
					}
				}

				CertFreeCertificateContext(pContext);
				CertCloseStore(hStore, 0);
			}

			// this is called before debug context is set and log ends up in log.log
			//nlinfo("Loaded %d certificates from '%s' certificate store", (int)CertList.size(), root);
		}
#endif

		void addCertificatesFromFile(const std::string &cert)
		{
			if (!isInitialized)
			{
				nlwarning("CURL not initialized! Check if there are another errors");
				return;
			}

			if (!isUsingOpenSSLBackend)
			{
				nlinfo("CURL not using OpenSSL backend! Unable to use custom certificates");
				return;
			}
			else
			{
				nlinfo("CURL using OpenSSL backend!");
			}

			// this file was already loaded
			if (std::find(CertList.begin(), CertList.end(), cert) != CertList.end()) return;

			// look for certificate in search paths
			string path = CPath::lookup(cert, false);

			if (path.empty())
			{
				nlwarning("Unable to find %s", cert.c_str());
				return;
			}

			nlinfo("CURL CA bundle '%s'", path.c_str());

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
							CertEntry entry;
							entry.cert = X509_dup(itmp->x509);
							entry.file = cert;
							entry.name = getCertName(entry.cert);

							CertList.push_back(entry);
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

	// cURL SSL certificate loading
	static CURLcode sslCtxFunction(CURL *curl, void *sslctx, void *parm)
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
					SX509Certificates::CertEntry entry = x509CertListManager.CertList[i];

					// add our certificate to this store
					if (X509_STORE_add_cert(x509store, entry.cert) == 0)
					{
						uint errCode = ERR_get_error();

						// ignore already in hash table errors
						if (ERR_GET_LIB(errCode) != ERR_LIB_X509 || ERR_GET_REASON(errCode) != X509_R_CERT_ALREADY_IN_HASH_TABLE)
						{
							ERR_error_string_n(errCode, errorBuffer, 1024);
							nlwarning("Error adding certificate %s: %s", entry.name.c_str(), errorBuffer);
							// There seems to be intermittent issues (on windows) where cert loading will fail for same 3 to 5 certs
							// with an 'SSL_shutdown while in init' error. It does not seem to be fatal for connection.
							//res = CURLE_SSL_CACERT;
						}
					}
					else
					{
						// nldebug("Added certificate %s", entry.name.c_str());
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

	// ***************************************************************************
	// static
	void CCurlCertificates::addCertificateFile(const std::string &cert)
	{
		x509CertListManager.addCertificatesFromFile(cert);
	}

	// ***************************************************************************
	// static
	void CCurlCertificates::useCertificates(CURL *curl)
	{
		// CURL must be valid, using OpenSSL backend and certificates must be loaded, else return
		if (!curl || !x509CertListManager.isInitialized || !x509CertListManager.isUsingOpenSSLBackend || x509CertListManager.CertList.empty()) return;

		curl_easy_setopt(curl, CURLOPT_SSLCERTTYPE, "PEM");

		// would allow to provide the CA in memory instead of using CURLOPT_CAINFO, but needs to include and link OpenSSL
		if (curl_easy_setopt(curl, CURLOPT_SSL_CTX_FUNCTION, &sslCtxFunction) != CURLE_OK)
		{
			nlwarning("Unable to support CURLOPT_SSL_CTX_FUNCTION, curl not compiled with OpenSSL ?");
		}

		// set both CURLOPT_CAINFO and CURLOPT_CAPATH to NULL to be sure we won't use default values (these files can be missing and generate errors)
		curl_easy_setopt(curl, CURLOPT_CAINFO, NULL);
		curl_easy_setopt(curl, CURLOPT_CAPATH, NULL);
	}

}// namespace

/* end of file */
