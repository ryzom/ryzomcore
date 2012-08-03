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

void CFileDepend::CDependency::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(CRC32);
	stream.serial(MacroPath);
}

void CFileDepend::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(CRC32);
	stream.serialCont(Dependencies);
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

void CFileOutput::serial(NLMISC::IStream &stream) throw (NLMISC::EStream)
{
	uint version = stream.serialVersion(1);
	stream.serial(BuildStart);
	stream.serialCont(MacroPaths);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

std::string CMetadataStorage::getStatusPath(const std::string &file)
{
	return CWorkspaceStorage::getMetaFilePath(file, PIPELINE_DATABASE_STATUS_SUFFIX);
}

bool CMetadataStorage::readStatus(CFileStatus &status, const std::string &metaPath)
{
	if (!NLMISC::CFile::fileExists(metaPath))
	{
		status.FirstSeen = 0;
		status.LastChangedReference = 0;
		status.LastFileSizeReference = ~0;
		status.LastUpdate = 0;
		status.CRC32 = 0;
		return false;
	}

	nlassert(!NLMISC::CFile::isDirectory(metaPath));

	NLMISC::CIFile is(metaPath, false);
	status.serial(is);
	is.close();

	return true;
}

void CMetadataStorage::writeStatus(const CFileStatus &status, const std::string &metaPath)
{
	NLMISC::COFile os(metaPath, false, false, true);
	const_cast<CFileStatus &>(status).serial(os);
	os.flush();
	os.close();
}

void CMetadataStorage::eraseStatus(const std::string &metaPath)
{
	NLMISC::CFile::deleteFile(metaPath);
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
	nldebug("Result metaPath: '%s'", resultPath.c_str());
	return resultPath;
}

void CMetadataStorage::readProcessResult(CProcessResult &result, const std::string &metaPath)
{
	if (!NLMISC::CFile::fileExists(metaPath))
	{
		nlwarning("Process running for the first time, this may take a long time");
		result.clear();
		return;
	}

	NLMISC::CIFile is(metaPath, false);
	result.serial(is);
	is.close();
}

void CMetadataStorage::writeProcessResult(const CProcessResult &result, const std::string &metaPath)
{
	NLMISC::COFile os(metaPath, false, false, true);
	const_cast<CProcessResult &>(result).serial(os);
	os.flush();
	os.close();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

std::string CMetadataStorage::getOutputPath(const std::string &file, const std::string &projectName, const std::string &pluginName)
{
	std::string lwProjectName = NLMISC::toLower(projectName);
	std::string lwPluginName = NLMISC::toLower(pluginName);
	return CWorkspaceStorage::getMetaFilePath(file, std::string(".") + lwProjectName + "." + lwPluginName + PIPELINE_DATABASE_OUTPUT_SUFFIX);
}

bool CMetadataStorage::readOutput(CFileOutput &output, const std::string &metaPath)
{
	if (!NLMISC::CFile::fileExists(metaPath))
	{
		output.BuildStart = 0;
		output.MacroPaths.clear();
		return false;
	}

	nlassert(!NLMISC::CFile::isDirectory(metaPath));

	NLMISC::CIFile is(metaPath, false);
	output.serial(is);
	is.close();

	return true;
}

void CMetadataStorage::writeOutput(const CFileOutput &output, const std::string &metaPath)
{
	NLMISC::COFile os(metaPath, false, false, true);
	const_cast<CFileOutput &>(output).serial(os);
	os.flush();
	os.close();
}

void CMetadataStorage::eraseOutput(const std::string &metaPath)
{
	NLMISC::CFile::deleteFile(metaPath);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

std::string CMetadataStorage::getDependPath(const std::string &file)
{
	return CWorkspaceStorage::getMetaFilePath(file, PIPELINE_DATABASE_DEPEND_SUFFIX);
}

bool CMetadataStorage::readDepend(CFileDepend &depend, const std::string &metaPath)
{
	if (!NLMISC::CFile::fileExists(metaPath))
	{
		depend.CRC32 = 0;
		depend.Dependencies.clear();
		return false;
	}

	nlassert(!NLMISC::CFile::isDirectory(metaPath));

	NLMISC::CIFile is(metaPath, false);
	depend.serial(is);
	is.close();

	return true;
}

void CMetadataStorage::writeDepend(const CFileDepend &depend, const std::string &metaPath)
{
	NLMISC::COFile os(metaPath, false, false, true);
	const_cast<CFileDepend &>(depend).serial(os);
	os.flush();
	os.close();
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

} /* namespace PIPELINE */

/* end of file */
