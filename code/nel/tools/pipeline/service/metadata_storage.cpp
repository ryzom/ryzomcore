/**
 * \file metadata_storage.cpp
 * \brief CMetadataStorage
 * \date 2012-07-30 14:31GMT
 * \author Jan Boon (Kaetemi)
 * CMetadataStorage
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU Affero General Public
 * License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 * 
 * You should have received a copy of the GNU Affero General Public
 * License along with RYZOM CORE PIPELINE.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "metadata_storage.h"

// STL includes

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/file.h>
#include <nel/misc/path.h>
#include <nel/misc/stream.h>

// Project includes

using namespace std;
// using namespace NLMISC;

namespace PIPELINE {

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

void CFileError::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(Project);
	stream.serial(Process);
	stream.serial(Message);
}

void CFileStatus::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(2);
	// if (version >= 3) stream.serial(LastRemoved); else LastRemoved = 0;
	stream.serial(FirstSeen);
	stream.serial(LastChangedReference);
	if (version >= 2) stream.serial(LastFileSizeReference); else LastFileSizeReference = 0;
	stream.serial(LastUpdate);
	stream.serial(CRC32);
}

void CFileRemove::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(Lost);
}

void CProcessResult::CFileResult::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(CRC32);
	stream.serial((uint8 &)Level); // test this :o)
}

void CProcessResult::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(BuildStart);
	stream.serialCont(MacroPaths);
	stream.serialCont(FileResults);
}

void CProcessResult::clear()
{
	BuildStart = 0;
	MacroPaths.clear();
	FileResults.clear();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

std::string CMetadataStorage::getStatusPath(const std::string &file)
{
	return CWorkspaceStorage::getMetaFilePath(file, PIPELINE_DATABASE_STATUS_SUFFIX);
}

bool CMetadataStorage::readStatus(CFileStatus &status, const std::string &path)
{
	if (!NLMISC::CFile::fileExists(path))
	{
		status.FirstSeen = 0;
		status.LastChangedReference = 0;
		status.LastFileSizeReference = ~0;
		status.LastUpdate = 0;
		status.CRC32 = 0;
		return false;
	}

	nlassert(!NLMISC::CFile::isDirectory(path));

	NLMISC::CIFile is(path, false);
	status.serial(is);
	is.close();

	return true;
}

void CMetadataStorage::writeStatus(const CFileStatus &status, const std::string &path)
{
	NLMISC::COFile os(path, false, false, true);
	const_cast<CFileStatus &>(status).serial(os);
	os.flush();
	os.close();
}

void CMetadataStorage::eraseStatus(const std::string &path)
{
	NLMISC::CFile::deleteFile(path);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

std::string CMetadataStorage::getResultPath(const std::string &projectName, const std::string &pluginName)
{
	std::string lwPluginName = NLMISC::toLower(pluginName);
	std::string resultPath = CWorkspaceStorage::getMetaDirectoryPath(
		CWorkspaceStorage::getProjectDirectory(projectName))
		+ lwPluginName + PIPELINE_DATABASE_RESULT_SUFFIX;
	nldebug("Result path: '%s'", resultPath.c_str());
	return resultPath;
}

void CMetadataStorage::readProcessResult(CProcessResult &result, const std::string &path)
{
	if (!NLMISC::CFile::fileExists(path))
	{
		nlwarning("Process running for the first time, this may take a long time");
		result.clear();
		return;
	}

	NLMISC::CIFile is(path, false);
	result.serial(is);
	is.close();
}

void CMetadataStorage::writeProcessResult(const CProcessResult &result, const std::string &path)
{
	NLMISC::COFile os(path, false, false, true);
	const_cast<CProcessResult &>(result).serial(os);
	os.flush();
	os.close();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

} /* namespace PIPELINE */

/* end of file */
