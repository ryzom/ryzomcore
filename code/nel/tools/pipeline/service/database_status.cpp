/**
 * \file database_status.cpp
 * \brief CDatabaseStatus
 * \date 2012-02-18 18:11GMT
 * \author Jan Boon (Kaetemi)
 * CDatabaseStatus
 */

/* 
 * Copyright (C) 2012  by authors
 * 
 * This file is part of RYZOM CORE PIPELINE.
 * RYZOM CORE PIPELINE is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation, either version 2 of
 * the License, or (at your option) any later version.
 * 
 * RYZOM CORE PIPELINE is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with RYZOM CORE PIPELINE; see the file COPYING.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#include <nel/misc/types_nl.h>
#include "database_status.h"

// STL includes
#include <boost/crc.hpp>

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/async_file_manager.h>
#include <nel/misc/path.h>
#include <nel/misc/file.h>
#include <nel/net/service.h>

// Project includes
#include "pipeline_service.h"

using namespace std;
using namespace NLMISC;

namespace PIPELINE {

namespace {

/// Input must be normalized path
bool isInDatabaseDirectoryFast(const std::string &path)
{
	return path.find(g_DatabaseDirectory) == 0;
}

/// Input must be normalized path
bool isInPipelineDirectoryFast(const std::string &path)
{
	return path.find(g_PipelineDirectory) == 0;
}

/// Input must be normalized path in database directory
std::string dropDatabaseDirectoryFast(const std::string &path)
{
	return path.substr(g_DatabaseDirectory.length());
}

/// Input must be normalized path in pipeline directory
std::string dropPipelineDirectoryFast(const std::string &path)
{
	return path.substr(g_PipelineDirectory.length());
}

/// Create status file path
std::string getStatusFilePath(const std::string &path)
{
	std::string stdPath = CPath::standardizePath(path, false);
	if (isInDatabaseDirectoryFast(stdPath))
	{
		std::string relPath = dropDatabaseDirectoryFast(stdPath);
		return g_PipelineDirectory + PIPELINE_DATABASE_STATUS_SUBDIR + relPath + PIPELINE_DATABASE_STATUS_SUFFIX;
	}
	else if (isInPipelineDirectoryFast(stdPath))
	{
		// TODO_TEST
		std::string relPath = dropPipelineDirectoryFast(stdPath);
		std::string::size_type slashPos = relPath.find_first_of('/');
		std::string proProName = relPath.substr(0, slashPos);
		std::string subPath = relPath.substr(slashPos);
		return g_PipelineDirectory + proProName + PIPELINE_DATABASE_STATUS_SUFFIX + subPath + PIPELINE_DATABASE_STATUS_SUFFIX;
	}
	else
	{
		nlerror("Path is not in database or pipeline");
		return path + PIPELINE_DATABASE_STATUS_SUFFIX;
	}
}

} /* anonymous namespace */

void CFileError::serial(NLMISC::IStream &stream)
{
	uint version = stream.serialVersion(1);
	stream.serial(Project);
	stream.serial(Process);
	stream.serial(Message);
}

void CFileStatus::serial(NLMISC::IStream &stream)
{
	uint version = stream.serialVersion(2);
	stream.serial(FirstSeen);
	stream.serial(LastChangedReference);
	if (version >= 2) stream.serial(LastFileSizeReference);
	stream.serial(LastUpdate);
	stream.serial(CRC32);
}

CDatabaseStatus::CDatabaseStatus()
{
	CFile::createDirectoryTree(g_PipelineDirectory + PIPELINE_DATABASE_STATUS_SUBDIR);
	CFile::createDirectoryTree(g_PipelineDirectory + PIPELINE_DATABASE_ERRORS_SUBDIR);
}

CDatabaseStatus::~CDatabaseStatus()
{
	
}

bool CDatabaseStatus::getFileStatus(CFileStatus &fileStatus, const std::string &filePath) const
{
	bool seemsValid = false;
	std::string stdPath = unMacroPath(filePath);
	std::string statusPath = getStatusFilePath(filePath);
	m_StatusMutex.lock_shared();
	if (CFile::fileExists(statusPath))
	{
		CIFile ifs(statusPath, false);
		fileStatus.serial(ifs);
		ifs.close();
		uint32 fmdt = CFile::getFileModificationDate(stdPath);
		uint32 fisz = CFile::getFileSize(stdPath);
		seemsValid =
			((fmdt == fileStatus.LastChangedReference)
			&& (fisz == fileStatus.LastFileSizeReference));
	}
	else
	{
		fileStatus.FirstSeen = 0;
		fileStatus.LastChangedReference = 0;
		fileStatus.LastFileSizeReference = ~0;
		fileStatus.LastUpdate = 0;
		fileStatus.CRC32 = 0;
	}
	m_StatusMutex.unlock_shared();
	return seemsValid;
}

namespace {

class CUpdateFileStatus : public IRunnable
{
public:
	virtual void getName(std::string &result) const 
	{ result = "CUpdateFileStatus"; }

	TFileStatusCallback Callback;
	std::string FilePath; // Standardized!
	boost::shared_mutex *StatusMutex;

	virtual void run()
	{
		CFileStatus fs;
		if (!g_IsExiting)
		{
			bool firstSeen = false;
			uint32 time = CTime::getSecondsSince1970();
			uint32 fmdt = CFile::getFileModificationDate(FilePath);
			std::string statusPath = getStatusFilePath(FilePath); // g_PipelineDirectory + PIPELINE_DATABASE_STATUS_SUBDIR + dropDatabaseDirectory(FilePath) + ".status";
			StatusMutex->lock_shared();
			if (CFile::fileExists(statusPath))
			{
				CIFile ifs(statusPath, false);
				fs.serial(ifs);
				ifs.close();
			}
			else
			{
				firstSeen = true;
				fs.LastChangedReference = 0;
				fs.LastFileSizeReference = ~0;
			}
			StatusMutex->unlock_shared();
			if (fs.LastChangedReference == fmdt && fs.LastFileSizeReference == CFile::getFileSize(FilePath))
			{
				nlinfo("Skipping already updated status, may have been queued twice (%s)", FilePath.c_str());
				if (firstSeen) nlerror("File first seen has same last changed time, not possible.");
			}
			else
			{
				if (firstSeen)
				{
					fs.FirstSeen = time;
					fs.CRC32 = 0;
					// nldebug("First seen file: '%s'", FilePath.c_str());
					
					// create dir
					CFile::createDirectoryTree(CFile::getPath(statusPath));
				}
				fs.LastChangedReference = fmdt;
				fs.LastUpdate = time;
				
				// nldebug("Calculate crc32 of file: '%s'", FilePath.c_str());
				{
					uint32 fisz = 0;
					CIFile ifs(FilePath, false);
					boost::crc_32_type result;
					while (!ifs.eof())
					{
						uint8 byte;
						ifs.serial(byte);
						result.process_byte(byte);
						++fisz;
					}
					ifs.close();
					fs.CRC32 = result.checksum();
					fs.LastFileSizeReference = fisz;
				}
				
				StatusMutex->lock();
				{
					COFile ofs(statusPath, false, false, true);
					fs.serial(ofs);
					ofs.flush();
					ofs.close();
				}
				StatusMutex->unlock();
			}
			Callback(FilePath, fs, true);
		}
		else
		{
			Callback(FilePath, fs, false);
		}	
		delete this;
	}
};

} /* anonymous namespace */

IRunnable *CDatabaseStatus::updateFileStatus(const TFileStatusCallback &callback, const std::string &filePath)
{
	if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
		return NULL;
	}

	CUpdateFileStatus *ufs = new CUpdateFileStatus();
	ufs->StatusMutex = &m_StatusMutex;
	ufs->FilePath = unMacroPath(filePath);
	ufs->Callback = callback;
	CAsyncFileManager::getInstance().addLoadTask(ufs);
	return ufs;
}

// ******************************************************************

namespace {

struct CDatabaseStatusUpdater
{
public:
	CCallback<void> Callback;

	CMutex Mutex;
	uint FilesRequested;
	uint FilesUpdated;
	bool Ready;
	bool CallbackCalled;

	std::vector<IRunnable *> RequestTasks;

	void fileUpdated(const std::string &filePath, const CFileStatus &fileStatus, bool success)
	{
		// warning: may be g_IsExiting during this callback!

		bool done = false;
		Mutex.enter();
		++FilesUpdated;
		if (FilesRequested == FilesUpdated && Ready && !CallbackCalled)
		{
			done = true;
			CallbackCalled = true;
		}
		Mutex.leave();

		if (done) doneRemove();

		if (g_IsExiting)
		{
			abortExit();
		}
	}

	void abortExit()
	{
		nlinfo("Abort database status update.");

		Mutex.enter();
		for (std::vector<IRunnable *>::iterator it = RequestTasks.begin(), end = RequestTasks.end(); it != end; ++it)
		{
			if (CAsyncFileManager::getInstance().deleteTask(*it))
			{
				++FilesUpdated;
				delete *it;
			}
		}

		bool done = false;
		if (!CallbackCalled && Ready)
		{
			done = true;
			CallbackCalled = true;
		}
		Mutex.leave();

		nlassert(FilesUpdated == FilesRequested);

		if (done) doneRemove();
	}

	void doneRemove()
	{
		nlinfo("Database status update done.");

		Callback();
		delete this;
	}
};

void updateDirectoryStatus(CDatabaseStatus* ds, CDatabaseStatusUpdater &updater, const std::string &dir, bool recurse);
void updatePathStatus(CDatabaseStatus* ds, CDatabaseStatusUpdater &updater, const std::string &subPath, bool recurse, bool recurseOnce)
{
	if (CFile::isDirectory(subPath)) // if the file is a directory!
	{
		if (recurse || recurseOnce)
			updateDirectoryStatus(ds, updater, subPath, recurse);
	}
	else
	{
		CFileStatus fileStatus;
		if (!ds->getFileStatus(fileStatus, subPath))
		{
			updater.Mutex.enter();
			if (!updater.CallbackCalled) // on abort.
			{
				++updater.FilesRequested;
				IRunnable *runnable = ds->updateFileStatus(TFileStatusCallback(&updater, &CDatabaseStatusUpdater::fileUpdated), subPath);
				updater.RequestTasks.push_back(runnable);
			}
			updater.Mutex.leave();
		}
	}
}

void updateDirectoryStatus(CDatabaseStatus* ds, CDatabaseStatusUpdater &updater, const std::string &dir, bool recurse)
{
	std::string dirPath = CPath::standardizePath(dir, true);
	std::vector<std::string> dirContents;

	CPath::getPathContent(dirPath, false, true, true, dirContents);
	
	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		const std::string subPath = *it;
		
		updatePathStatus(ds, updater, subPath, recurse, false);
		
		if (g_IsExiting)
			return;
	}
}

} /* anonymous namespace */

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback)
{
	if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
	}

	std::vector<std::string> paths;
	paths.push_back(g_DatabaseDirectory);
	updateDatabaseStatus(callback, paths, false, true);
}

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback, const std::vector<std::string> &paths, bool wait, bool recurse)
{
	if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
	}

	CDatabaseStatusUpdater *updater = new CDatabaseStatusUpdater();
	updater->Callback = callback;
	updater->FilesRequested = 0;
	updater->FilesUpdated = 0;
	updater->Ready = false;
	updater->CallbackCalled = false;
	
	nlinfo("Starting iteration through database, queueing file status updates.");
	
	for (std::vector<std::string>::const_iterator it = paths.begin(), end = paths.end(); it != end; ++it)
		updatePathStatus(this, *updater, unMacroPath(*it), recurse, true);
	
	nlinfo("Iteration through database, queueing file status updates complete.");
	
	bool done = false;
	updater->Mutex.enter();
	updater->Ready = true;
	if (updater->FilesRequested == updater->FilesUpdated && !updater->CallbackCalled)
	{
		done = true;
		updater->CallbackCalled = true;
	}
	updater->Mutex.leave();
	
	if (done) updater->doneRemove();

	if (wait)
	{
		// UGLY_CODE ->
		while (!updater->CallbackCalled)
		{
			nlSleep(10);
		}
		// <- UGLY_CODE
	}
}

// ******************************************************************

} /* namespace PIPELINE */

/* end of file */
