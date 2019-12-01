// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2014-2019  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

// Project includes
#include <nel/misc/streamed_package_manager.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/sha1.h>

namespace NLMISC
{

NLMISC_SAFE_SINGLETON_IMPL(CStreamedPackageManager);

CStreamedPackageManager::CStreamedPackageManager() : Provider(NULL)
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

	IStreamedPackageProvider *provider = Provider;
	if (!provider)
	{
		nlerrornoex("No streamed package provider was set");
		return false;
	}

	TEntries::iterator it = m_Entries.find(fileName);
	if (it == m_Entries.end())
	{
		return false;
	}
	const CStreamedPackage::CEntry *entry = it->second;

	return provider->getFile(filePath, entry->Hash, fileName);

	return true;
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
