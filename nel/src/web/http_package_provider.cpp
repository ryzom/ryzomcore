// NeL - MMORPG Framework <https://wiki.ryzom.dev/>
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
#include <nel/web/http_package_provider.h>

// Project includes
#include <nel/misc/streamed_package.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/seven_zip.h>
#include <nel/misc/sha1.h>

using namespace std;
using namespace NLMISC;
using namespace NLWEB;

namespace NLWEB
{

CHttpPackageProvider::CHttpPackageProvider()
{
	// init
}

CHttpPackageProvider::~CHttpPackageProvider()
{
	// release
}

bool CHttpPackageProvider::getFile(std::string &filePath, const CHashKey &hash, const std::string &name)
{
	CStreamedPackage::makePath(filePath, hash);
	std::string downloadUrlFile = filePath + ".lzma";
	filePath = Path + filePath;
	std::string downloadPath = filePath + ".download." + toString(rand() * rand());

	std::string storageDirectory = CFile::getPath(downloadPath);
	CFile::createDirectoryTree(storageDirectory);
	/*if (!CFile::isDirectory(storageDirectory) || !CFile::createDirectoryTree(storageDirectory))
	{
		nldebug("Unable to create directory '%s'", storageDirectory.c_str());
		return false;
	}*/

	// download
	for (;;)
	{
		if (CFile::fileExists(filePath))
			return true;

		std::string downloadUrl = Hosts[rand() % Hosts.size()] + downloadUrlFile;
		nldebug("Download streamed package '%s' from '%s'", name.c_str(), downloadUrl.c_str());

		FILE *fp = fopen(downloadPath.c_str(), "wb");
		if (fp == NULL)
		{
			nldebug("Unable to create file '%s' for '%s'", downloadPath.c_str(), name.c_str());
			return false;
		}

		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, downloadUrl.c_str());
			if (downloadUrl.length() > 8 && (downloadUrl[4] == 's' || downloadUrl[4] == 'S')) // 01234 https
			{
				// Don't need to verify, since we check the hash
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
			}
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_FILE, fp);
			res = curl_easy_perform(curl);
			long r;
			curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &r);
			curl_easy_cleanup(curl);

			bool diskFull = ferror(fp) && errno == 28 /*ENOSPC*/;
			fclose(fp);

			if (diskFull)
			{
				CFile::deleteFile(downloadPath);
				throw EDiskFullError(downloadPath);
			}

			if (res != CURLE_OK || r < 200 || r >= 300)
			{
				CFile::deleteFile(downloadPath);
				nldebug("Download failed '%s', retry in 1s", downloadUrl.c_str());
				nlSleep(1000);
				continue;
			}
		}
		else
		{
			nldebug("Curl initialize failed");
			fclose(fp);
			CFile::deleteFile(downloadPath);
			return false;
		}

		// ok!
		break;
	}

	// extract into file
	std::string unpackPath = filePath + ".extract." + toString(rand() * rand());

	CHashKey outHash;
	if (!unpackLZMA(downloadPath, unpackPath, outHash))
	{
		CFile::deleteFile(downloadPath);
		return false;
	}
	CFile::deleteFile(downloadPath);

	if (!(outHash == hash))
	{
		std::string wantHashS = hash.toString();
		std::string outHashS = outHash.toString();
		nlwarning("Invalid SHA1 hash for file '%s', download has hash '%s'", wantHashS.c_str(), outHashS.c_str());
		return false;
	}

	if (!CFile::moveFile(filePath.c_str(), unpackPath.c_str()))
	{
		nldebug("Failed moving '%s' to '%s'", unpackPath.c_str(), filePath.c_str());
		// in case downloaded from another thread
		return CFile::fileExists(filePath);
	}

	return true;
}

} /* namespace NLMISC */

/* end of file */
