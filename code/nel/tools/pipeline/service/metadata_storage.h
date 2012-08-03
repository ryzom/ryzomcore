/**
 * \file metadata_storage.h
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

#ifndef PIPELINE_METADATA_STORAGE_H
#define PIPELINE_METADATA_STORAGE_H
#include <nel/misc/types_nl.h>

// STL includes

// NeL includes
#include <vector>

// Project includes
#include "workspace_storage.h"

namespace NLMISC {
	class IStream;
	struct EStream;
}

namespace PIPELINE {

// Status is generated CRC32 for reference.
// Errors are errors caused by using this file as an input or output file.

enum TFileState
{
	STATE_UNKNOWN = 0,
	STATE_SUCCESS = 1,
	STATE_WARNING = 2,
	STATE_ERROR = 3,
	STATE_REMOVAL = 4,
};

/// Suffix for metafiles that contain the CRC32 etc
#define PIPELINE_DATABASE_STATUS_SUFFIX ".status"
struct CFileStatus
{
public:
	// uint32 LastRemoved; // The last time this file was removed, purely informational (at the moment) because we can detect past removal and re-addition by comparing FirstSeen with our reference build time as well.
	uint32 FirstSeen; // The time when this status file was first created (if the file was removed before this means the time when the file returned).
	uint32 LastChangedReference; // The modification date value read when the CRC32 was calculated.
	uint32 LastFileSizeReference; // The filesize when the CRC32 was calculated.
	uint32 LastUpdate; // The start time when the CRC32 was calculated.
	uint32 CRC32;

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
};

/// Suffix for metafiles that contain error info on database files
#define PIPELINE_DATABASE_ERRORS_SUFFIX ".errors"
struct CFileError
{
public:
	uint32 Time; // The time when this error occured.
	// TFileState Level; // Success, Warning, Error, Removal
	std::string Project;
	std::string Process;
	// std::string Plugin;
	std::string Message;

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
};

/// Errors set by a process when the file causes a build failure.
typedef std::vector<CFileError> CFileErrors;

/// Suffix for metafiles that refer to a previously known file that no longer exists
#define PIPELINE_DATABASE_REMOVE_SUFFIX ".remove"
struct CFileRemove
{
public:
	uint32 Lost; // The time when it was noticed the file was removed.

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
};

/// Suffix for metafiles that contain dependencies for a file
#define PIPELINE_DATABASE_DEPEND_SUFFIX ".depend"
struct CFileDepend
{
public:
	uint32 CRC32; // Checksum of the current file
	struct CDependency
	{
		std::string MacroPath;
		uint32 CRC32;

		void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
	};
	std::vector<CDependency> Dependencies;

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
};

/// Suffix for metafiles that contain the result of the last build of a process
#define PIPELINE_DATABASE_RESULT_SUFFIX ".result"
struct CProcessResult
{
public:
	// Note: this file may have been erased on build start in case something went wrong!
	// In that case, the last successfulbuild start is 0, and no output files will be known.
	// This is the same situation as if the project never built before.
	// It must be handled sanely.
	// This file is only stored when the build completed successfully.
	// If it did not, bad output files can be noticed by having a different CRC32.
	uint32 BuildStart;
	std::vector<std::string> MacroPaths;
	struct CFileResult
	{
		uint32 CRC32;
		TFileState Level;

		void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
	};
	std::vector<CFileResult> FileResults;

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
	void clear();
};

/// Suffix for metafiles that contain the list of output files of an input file when it was last built (what happens to this meta when the file fails build? - erase it for now... forces a rebuild)
#define PIPELINE_DATABASE_OUTPUT_SUFFIX ".output"
struct CFileOutput
{
public:
	uint32 BuildStart;
	std::vector<std::string> MacroPaths;

	void serial(NLMISC::IStream &stream) throw (NLMISC::EStream);
};

/**
 * \brief CMetadataStorage
 * \date 2012-07-30 14:31GMT
 * \author Jan Boon (Kaetemi)
 * CMetadataStorage
 */
class CMetadataStorage
{
public:

	// Note: Use the functions provided by CDatabaseStatus for manipulating status files.
	/// Format like .../something.somedirectory.meta/path/file.status
	static std::string getStatusPath(const std::string &file);
	static bool readStatus(CFileStatus &status, const std::string &metaPath);
	static void writeStatus(const CFileStatus &status, const std::string &metaPath);
	static void eraseStatus(const std::string &metaPath);

	/// Format like .../something.somedirectory.meta/path/file.remove
	// static std::string getRemovePath(const std::string &file);
	// static bool readRemove(CFileRemove &remove, const std::string & path);
	// static void createRemove(const CFileRemove &remove, const std::string &path); // Remove cannot be modified after creation, only erased.
	// static void eraseRemove(const std::string &path);

	static std::string getDependPath(const std::string &file);
	static bool readDepend(CFileDepend &depend, const std::string &metaPath);
	static void writeDepend(const CFileDepend &depend, const std::string &metaPath);

	// Pathname for result metadata is like .../project.projectname.meta/pluginname.result
	static std::string getResultPath(const std::string &projectName, const std::string &pluginName);
	static void readProcessResult(CProcessResult &result, const std::string &metaPath);
	static void writeProcessResult(const CProcessResult &result, const std::string &metaPath);

	// Pathname for the output metadata file, like .../something.somedirectory.meta/path/file.projectname.pluginname.output
	static std::string getOutputPath(const std::string &file, const std::string &projectName, const std::string &pluginName);
	static bool readOutput(CFileOutput &output, const std::string &metaPath);
	static void writeOutput(const CFileOutput &output, const std::string &metaPath);
	static void eraseOutput(const std::string &metaPath);

}; /* class CMetadataStorage */

} /* namespace PIPELINE */

#endif /* #ifndef PIPELINE_METADATA_STORAGE_H */

/* end of file */
