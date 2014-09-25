// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014  Jan BOON (jan.boon@kaetemi.be)
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

#include "stdmisc.h"

// 3rd Party includes
#include <curl/curl.h>
#include <seven_zip/7zCrc.h>
#include <seven_zip/7zIn.h>
#include <seven_zip/7zExtract.h>
#include <seven_zip/LzmaDecode.h>

// Project includes
#include <nel/misc/streamed_package_manager.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>

namespace NLMISC {

NLMISC_SAFE_SINGLETON_IMPL(CStreamedPackageManager);

CStreamedPackageManager::CStreamedPackageManager()
{
	// init
}

CStreamedPackageManager::~CStreamedPackageManager()
{
	// release
}

bool CStreamedPackageManager::loadPackage(const std::string &package)
{
	nldebug("Load package '%s'", package.c_str());

	std::string packname = NLMISC::toLower(CFile::getFilename(package));
	m_Packages[packname] = CStreamedPackage();
	std::map<std::string, CStreamedPackage>::iterator it = m_Packages.find(packname);
	CStreamedPackage &p = it->second;

	try
	{
		CIFile fi;
		fi.open(package);
		fi.serial(p);
	}
	catch (Exception &e)
	{
		nlerror("Package serial exception: '%s'", e.what());
		m_Packages.erase(it);
		return false;
	}

	for (CStreamedPackage::TEntries::const_iterator it(p.Entries.begin()), end(p.Entries.end()); it != end; ++it)
	{
		const CStreamedPackage::CEntry &entry = (*it);
		m_Entries[entry.Name] = &entry;
	}

	return true;
}

void CStreamedPackageManager::list(std::vector<std::string> &fileNames, const std::string &package)
{
	// nldebug("List package '%s'", package.c_str());

	std::map<std::string, CStreamedPackage>::iterator it = m_Packages.find(package);
	CStreamedPackage &p = it->second;

	for (CStreamedPackage::TEntries::const_iterator it(p.Entries.begin()), end(p.Entries.end()); it != end; ++it)
	{
		const CStreamedPackage::CEntry &entry = (*it);
		fileNames.push_back(entry.Name);
	}
}

void CStreamedPackageManager::unloadAll()
{
	m_Packages.clear();
	m_Entries.clear();
}

bool CStreamedPackageManager::getFile(std::string &filePath, const std::string &fileName)
{
	// nldebug("Get file path for streamed file '%s'", fileName.c_str());

	TEntries::iterator it = m_Entries.find(fileName);
	if (it == m_Entries.end())
		return false;
	const CStreamedPackage::CEntry *entry = it->second;

	CStreamedPackage::makePath(filePath, entry->Hash);
	std::string downloadUrlFile = filePath + ".lzma";
	filePath = Path + filePath;
	std::string downloadPath = filePath + ".download." + toString(rand());

	std::string storageDirectory = CFile::getPath(downloadPath);
	CFile::createDirectoryTree(storageDirectory);
	/*if (!CFile::isDirectory(storageDirectory) || !CFile::createDirectoryTree(storageDirectory))
	{
		nldebug("Unable to create directory '%s'", storageDirectory.c_str());
		return false;
	}*/

	// download
	for (; ; )
	{
		if (CFile::fileExists(filePath))
			return true;

		std::string downloadUrl = Hosts[rand() % Hosts.size()] + downloadUrlFile;
		nldebug("Download streamed package '%s' from '%s'", fileName.c_str(), downloadUrl.c_str());

		FILE *fp = fopen(downloadPath.c_str(), "wb");
		if (fp == NULL)
		{
			nldebug("Unable to create file '%s' for '%s'", downloadPath.c_str(), fileName.c_str());
			return false;
		}

		CURL *curl;
		CURLcode res;
		curl = curl_easy_init();
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, downloadUrl.c_str());
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

	// read into memory :(
	std::string unpackPath = filePath + ".extract." + toString(rand());

	std::vector<uint8> inBuffer;
	{
		CIFile inStream(downloadPath);
		uint32 inSize = inStream.getFileSize();
		inBuffer.resize(inSize);
		inStream.serialBuffer(&inBuffer[0], inSize);
	}
	CFile::deleteFile(downloadPath);

	if (inBuffer.size() < LZMA_PROPERTIES_SIZE + 8)
	{
		nlwarning("Invalid file size %u, too small file '%s'", inBuffer.size(), downloadPath.c_str());
		return false;
	}

	// extract
	{
		CLzmaDecoderState state;
		uint8 *pos = &inBuffer[0];
		int ret = LzmaDecodeProperties(&state.Properties, (unsigned char *)pos, LZMA_PROPERTIES_SIZE);
		if (ret != 0)
		{
			nlwarning("Failed to decode lzma properies in file '%s'", downloadPath.c_str());
			return false;
		}

		// FROM login_patch.cpp
		// alloc the probs, making sure they are deleted in function exit
		size_t nbProb = LzmaGetNumProbs(&state.Properties);
		std::vector<CProb> probs;
		probs.resize(nbProb);
		state.Probs = &probs[0];

		pos += LZMA_PROPERTIES_SIZE;

		// read the output file size
		size_t fileSize = 0;
		for (int i = 0; i < 8; i++)
		{
			//Byte b;
			if (pos >= &inBuffer[0] + inBuffer.size())
			{
				nlerror("pos >= inBuffer.get() + inSize");
				return false;
			}
			fileSize |= ((UInt64)*pos++) << (8 * i);
		}

		nlassert(fileSize == entry->Size);

		SizeT outProcessed = 0;
		SizeT inProcessed = 0;
		// allocate the output buffer :(
		std::vector<uint8> outBuffer;
		outBuffer.resize(fileSize);
		// decompress the file in memory
		ret = LzmaDecode(&state, (unsigned char *)pos, (SizeT)(inBuffer.size() - (pos - &inBuffer[0])), &inProcessed, (unsigned char*)&outBuffer[0], (SizeT)fileSize, &outProcessed);
		if (ret != 0 || outProcessed != fileSize)
		{
			nlwarning("Failed to decode lzma file '%s'", downloadPath.c_str());
			return false;
		}

		{
			COFile outStream(unpackPath);
			outStream.serialBuffer(&outBuffer[0], (uint)fileSize);
		}

		if (!CFile::moveFile(filePath.c_str(), unpackPath.c_str()))
		{
			nldebug("Failed moving '%s' to '%s'", unpackPath.c_str(), filePath.c_str());
			// in case downloaded from another thread
			return CFile::fileExists(filePath);
		}

		return true;
	}
}

bool CStreamedPackageManager::getFileSize(uint32 &fileSize, const std::string &fileName)
{
	// nldebug("Get file size for streamed file '%s'", fileName.c_str());
	TEntries::iterator it = m_Entries.find(fileName);
	if (it == m_Entries.end())
		return false;
	fileSize = it->second->Size;
	return true;
}

} /* namespace NLMISC */

/* end of file */
