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

// Project includes
#include "pipeline_service.h"

using namespace std;
using namespace NLMISC;

namespace PIPELINE {

namespace {

std::string dropDatabaseDirectory(const std::string &path)
{
	if (path.find(g_DatabaseDirectory) == 0)
	{
		return path.substr(g_DatabaseDirectory.length());
	}
	else
	{
		nlerror("Path is not in database.");
		return path;
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
	uint version = stream.serialVersion(1);
	stream.serial(FirstSeen);
	stream.serial(LastChanged);
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
	// mutex here when reading
	return false;
}

namespace {

class CUpdateFileStatus : public IRunnable
{
public:
	virtual void getName(std::string &result) const 
	{ result = "CUpdateFileStatus"; }

	TFileStatusCallback Callback;
	std::string FilePath;
	CMutex *StatusMutex;

	virtual void run()
	{
		nldebug("Run this");
		bool firstSeen = false;
		uint32 time = CTime::getSecondsSince1970();
		std::string statusPath = g_PipelineDirectory + PIPELINE_DATABASE_STATUS_SUBDIR + dropDatabaseDirectory(FilePath) + ".status";
		CFileStatus fs;
		StatusMutex->enter();
		if (CFile::fileExists(statusPath))
		{
			CIFile ifs(statusPath, false);
			fs.serial(ifs);
			ifs.close();
		}
		else
		{
			firstSeen = true;
		}
		StatusMutex->leave();
		if (firstSeen)
		{
			fs.FirstSeen = time;
			fs.CRC32 = 0;
			nldebug("First seen file: '%s'", FilePath.c_str());
			
			// create dir
			CFile::createDirectoryTree(g_PipelineDirectory + PIPELINE_DATABASE_STATUS_SUBDIR + dropDatabaseDirectory(CFile::getPath(FilePath)));
		}
		fs.LastChanged = CFile::getFileModificationDate(FilePath);
		fs.LastUpdate = time;
		
		nldebug("Calculate crc32 of file: '%s'", FilePath.c_str());
		nlSleep(1000);
		// calculate crc32 etcetera etcetera
		
		StatusMutex->enter();
		{
			COFile ofs(statusPath, false, false, true);
			fs.serial(ofs);
			ofs.flush();
			ofs.close();
		}
		StatusMutex->leave();
		nldebug("Callback");
		Callback(FilePath, fs);		
		nldebug("Delete this");
		delete this;
	}
};

} /* anonymous namespace */

void CDatabaseStatus::updateFileStatus(const TFileStatusCallback &callback, const std::string &filePath)
{
	if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
		return;
	}

	CUpdateFileStatus *ufs = new CUpdateFileStatus();
	ufs->StatusMutex = &m_StatusMutex;
	ufs->FilePath = filePath;
	ufs->Callback = callback;
	CAsyncFileManager::getInstance().addLoadTask(ufs);
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

	void fileUpdated(const std::string &filePath, const CFileStatus &fileStatus)
	{
		nldebug("File updated callback");

		bool done = false;
		Mutex.enter();
		++FilesUpdated;
		if (FilesRequested == FilesUpdated && Ready && !CallbackCalled)
		{
			done = true;
			CallbackCalled = true;
		}
		Mutex.leave();

		if (done) Callback();
	}
};

void updateDirectoryStatus(CDatabaseStatus* ds, CDatabaseStatusUpdater &updater, const std::string &dir)
{
	std::string dirPath = CPath::standardizePath(dir, true);
	std::vector<std::string> dirContents;

	CPath::getPathContent(dirPath, false, true, true, dirContents);
	
	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		const std::string subPath = *it;
		
		if (CFile::isDirectory(subPath)) // if the file is a directory!
		{
			updateDirectoryStatus(ds, updater, subPath);
		}
		else
		{
			updater.Mutex.enter();
			++updater.FilesRequested;
			updater.Mutex.leave();
			
			CFileStatus fileStatus;
			if (!ds->getFileStatus(fileStatus, subPath))
			{
				ds->updateFileStatus(TFileStatusCallback(&updater, &CDatabaseStatusUpdater::fileUpdated), subPath);
			}
		}
	}
}

} /* anonymous namespace */

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback)
{
	CDatabaseStatusUpdater updater;
	updater.Callback = callback;
	updater.FilesRequested = 0;
	updater.FilesUpdated = 0;
	updater.Ready = false;
	updater.CallbackCalled = false;

	nlinfo("Starting iteration through database, queueing file status updates.");

	// recursive loop
	updateDirectoryStatus(this, updater, g_DatabaseDirectory);

	nlinfo("Iteration through database, queueing file status updates complete.");
		
	bool done = false;
	updater.Mutex.enter();
	updater.Ready = true;
	if (updater.FilesRequested == updater.FilesUpdated && !updater.CallbackCalled)
	{
		done = true;
		updater.CallbackCalled = true;
	}
	updater.Mutex.leave();
	
	if (done) updater.Callback();
}

// ******************************************************************

} /* namespace PIPELINE */

/* end of file */
