/**
 * \file database_status.h
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

#ifndef PIPELINE_DATABASE_STATUS_H
#define PIPELINE_DATABASE_STATUS_H
#include <nel/misc/types_nl.h>

// STL includes
#include <string>
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

// NeL includes
#include <nel/misc/reader_writer.h>
#include <nel/misc/stream.h>

// Project includes
#include "callback.h"
#include "metadata_storage.h"

namespace NLMISC {
	class IRunnable;
}

namespace PIPELINE {

typedef CCallback<void, const std::string &/*filePath*/, const CFileStatus &/*fileStatus*/, bool /*success*/> TFileStatusCallback;

/**
 * \brief CDatabaseStatus
 * \date 2012-02-18 18:11GMT
 * \author Jan Boon (Kaetemi)
 * CDatabaseStatus
 */
class CDatabaseStatus
{
protected:
	mutable boost::shared_mutex m_StatusMutex;
	mutable boost::mutex m_ErrorMutex;
	
public:
	CDatabaseStatus();
	virtual ~CDatabaseStatus();
	
	/// Tries to read the last file status. Return false if the status is invalid. Call updateFileStatus if the result is false to update asynchronously.
	bool getFileStatus(CFileStatus &fileStatus, const std::string &filePath) const;
	/// Updates a file status synchronously. Returns the result in metaStatus. filepPath is NOT a macro path.
	bool updateFileStatus(CFileStatus &metaStatus, const std::string &filePath);
	/// Updates the file status asynchronously. Warning: If g_IsExiting during callback then update likely did not happen.
	NLMISC::IRunnable *updateFileStatus(const TFileStatusCallback &callback, const std::string &filePath);
	/// Runs an update of the complete {{DatabaseDirectory}} status asynchronously. Warning: If g_IsExiting during callback then update is incomplete. Callback is always called when done (or failed).
	void updateDatabaseStatus(const CCallback<void> &callback);
	/// Runs an update of the file status of given paths asynchronously. Warning: If g_IsExiting during callback then update is incomplete. Callback is always called when done (or failed). Do NOT use the wait parameter. Do NOT use recurse, please. Recurse directories beforehand. Paths may contain db and pl macros.
	void updateDatabaseStatus(const CCallback<void> &callback, const TFileStatusCallback &fileStatusCallback, const std::vector<std::string> &paths, bool wait = false, bool recurse = false);
	/// Gets the last file statuses of given paths in a map. Directories are scanned for files, non recursively. Returns false if one of the statuses is bad (not updated; file changed inbetween). Considered as build error.
	// bool getFileStatus(std::map<std::string, CFileStatus> &fileStatusMap, std::map<std::string, CFileRemove> &fileRemoveMap, const std::vector<std::string> &paths);
	/// Gets all removed files
	bool getRemoved(std::map<std::string, CFileRemove> &fileRemoveMap, const std::vector<std::string> &paths);

	void getFileErrors(CFileErrors &fileErrors, const std::string &filePath, uint32 newerThan = 0) const;
	void addFileError(const std::string &filePath, const CFileError &fileError);
	
}; /* class CDatabaseStatus */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_DATABASE_STATUS_H */

/* end of file */
