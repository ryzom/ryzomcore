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

// NeL includes
// #include <nel/misc/debug.h>
#include <nel/misc/async_file_manager.h>
#include <nel/misc/path.h>

// Project includes

using namespace std;
using namespace NLMISC;

namespace PIPELINE {

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
	
}

CDatabaseStatus::~CDatabaseStatus()
{
	
}

bool CDatabaseStatus::getFileStatus(CFileStatus &fileStatus, const std::string &filePath) const
{
	return false;
}

void CDatabaseStatus::updateFileStatus(const TFileStatusCallback &callback, const std::string &filePath)
{
	
}

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

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback)
{
	CDatabaseStatusUpdater updater;
	updater.Callback = callback;
	updater.FilesRequested = 0;
	updater.FilesUpdated = 0;
	updater.Ready = false;
	updater.CallbackCalled = false;

	// recursive loop ...
	{
		updater.Mutex.enter();
		++updater.FilesRequested;
		updater.Mutex.leave();
		updateFileStatus(TFileStatusCallback(&updater, &CDatabaseStatusUpdater::fileUpdated), "");
	}
	
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

} /* namespace PIPELINE */

/* end of file */
