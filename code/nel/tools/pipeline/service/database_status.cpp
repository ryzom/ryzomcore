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
bool isInRootDirectoryFast(std::string &rootDirectoryName, std::string &rootDirectoryPath, const std::string &path)
{
	//return path.find(g_DatabaseDirectory) == 0;
	CConfigFile::CVar &rootDirectories = NLNET::IService::getInstance()->ConfigFile.getVar("RootDirectories");
	for (uint i = 0; i < rootDirectories.size(); ++i)
	{
		rootDirectoryName = rootDirectories.asString(i);
		CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(rootDirectoryName);
		rootDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(rootDirectoryPath) == 0) return true;
	}
	return false;
}

bool isInSheetsDirectoryFast(std::string &sheetDirectoryName, std::string &sheetDirectoryPath, const std::string &path)
{
	{
		sheetDirectoryName = "WorkspaceDfnDirectory";
		CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(sheetDirectoryName);
		sheetDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(sheetDirectoryPath) == 0) return true;
	}
	{
		sheetDirectoryName = "WorkspaceSheetDirectory";
		CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(sheetDirectoryName);
		sheetDirectoryPath = standardizePath(dir.asString(), true);
		if (path.find(sheetDirectoryPath) == 0) return true;
	}
	return false;
}

/// Input must be normalized path
bool isInWorkspaceDirectoryFast(const std::string &path)
{
	return path.find(g_WorkDir) == 0;
}

/// Input must be normalized path in database directory
std::string dropRootDirectoryFast(const std::string &path, const std::string &rootDirectoryPath)
{
	return path.substr(rootDirectoryPath.length());
}

/// Input must be normalized path in sheets directory
std::string dropSheetDirectoryFast(const std::string &path, const std::string &sheetDirectoryPath)
{
	return path.substr(sheetDirectoryPath.length());
}

/// Input must be normalized path in pipeline directory
std::string dropWorkspaceDirectoryFast(const std::string &path)
{
	return path.substr(g_WorkDir.length());
}

} /* anonymous namespace */

std::string getMetaFilePath(const std::string &path, const std::string &dotSuffix)
{
	std::string stdPath = standardizePath(path, false);
	if (isInWorkspaceDirectoryFast(stdPath))
	{
		// TODO_TEST
		std::string relPath = dropWorkspaceDirectoryFast(stdPath);
		std::string::size_type slashPos = relPath.find_first_of('/');
		std::string proProName = relPath.substr(0, slashPos);
		std::string subPath = relPath.substr(slashPos);
		return g_WorkDir + proProName + PIPELINE_DATABASE_META_SUFFIX + subPath + dotSuffix;
	}
	else
	{
		std::string rootDirectoryName;
		std::string rootDirectoryPath;
		if (isInSheetsDirectoryFast(rootDirectoryName, rootDirectoryPath, stdPath))
		{
			std::string relPath = dropSheetDirectoryFast(stdPath, rootDirectoryPath);
			return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_SHEET + NLMISC::toLower(rootDirectoryName) + PIPELINE_DATABASE_META_SUFFIX + "/" + relPath + dotSuffix;
		}
		else
		{
			if (isInRootDirectoryFast(rootDirectoryName, rootDirectoryPath, stdPath))
			{
				std::string relPath = dropRootDirectoryFast(stdPath, rootDirectoryPath);
				return g_WorkDir + PIPELINE_DIRECTORY_PREFIX_ROOT + NLMISC::toLower(rootDirectoryName) + PIPELINE_DATABASE_META_SUFFIX + "/" + relPath + dotSuffix;
			}
			else
			{
				nlerror("Path is not in database or pipeline (%s)", path.c_str());
				return path + dotSuffix;
			}
		}
	}
}

namespace {

/// Create status file path
std::string getStatusFilePath(const std::string &path)
{
	return getMetaFilePath(path, PIPELINE_DATABASE_STATUS_SUFFIX);
}

} /* anonymous namespace */

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

CDatabaseStatus::CDatabaseStatus()
{
	//CFile::createDirectoryTree(g_WorkspaceDirectory + PIPELINE_DATABASE_STATUS_SUBDIR);
	//CFile::createDirectoryTree(g_WorkspaceDirectory + PIPELINE_DATABASE_ERRORS_SUBDIR);
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
		nlassert(!CFile::isDirectory(filePath));
		
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
		// fileStatus.LastRemoved = 0;
		fileStatus.FirstSeen = 0;
		fileStatus.LastChangedReference = 0;
		fileStatus.LastFileSizeReference = ~0;
		fileStatus.LastUpdate = 0;
		fileStatus.CRC32 = 0;
	}
	m_StatusMutex.unlock_shared();
	return seemsValid;
}

bool CDatabaseStatus::getFileStatus(std::map<std::string, CFileStatus> &fileStatusMap, std::map<std::string, CFileRemove> &fileRemoveMap, const std::vector<std::string> &paths)
{
	// ARE THE PATHS UNMACRO'D OR NOT???

	// nlassert(false); // i don't think this is used? (yet?) (maybe for slave?)
	// probably just some ghost code spooking around.

	// this code shall be used by the slave to get all the infos
	// in the slave it will compare the returned data for example with the last successful build time and return added/changed/removed to the plugin

	for (std::vector<std::string>::const_iterator it = paths.begin(), end = paths.end(); it != end; ++it)
	{
		std::string path = *it; // assume already unmacro'd!
		
		if (CFile::isExists(path))
		{
			if (CFile::isDirectory(path)) // perhaps it is possible to check only on / assuming it's properly formatted!!
			{
				nlassert(path.find_last_of('/') == path.size() - 1); // ensure sanity

				// std::string dirPath = standardizePath(path, true);
				std::string &dirPath = path;
				std::vector<std::string> dirContents;

				CPath::getPathContent(dirPath, false, false, true, dirContents); // get only files, no dirs
				
				for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
				{
					const std::string &subPath = *it;
					
					CFileStatus fs;
					if (!getFileStatus(fs, subPath))
					{
						// bad status, data corruption // TODO_PROCESS_ERROR
						nlwarning("Invalid status for '%s'!", subPath.c_str());
						return false; 
					}
					fileStatusMap[subPath] = fs; // i'll assume macropath might be easiest
					
					// TODO_PROCESS_ERROR_EXIT
					if (g_IsExiting)
						return false;
				}

				std::string dirPathMeta = getMetaFilePath(dirPath, "");
				std::vector<std::string> dirContentsMeta;

				CPath::getPathContent(dirPathMeta, false, false, true, dirContentsMeta);

				for (std::vector<std::string>::iterator it = dirContentsMeta.begin(), end = dirContentsMeta.end(); it != end; ++it)
				{
					const std::string &subPath = *it;
					
					std::string::size_type removePos = subPath.find(PIPELINE_DATABASE_REMOVE_SUFFIX);

					if (removePos != std::string::npos)
					{
						std::string subPathFilename = CFile::getFilename(subPath.substr(0, removePos));
						std::string subPathOrig = dirPath + subPathFilename;
						
						nldebug("Found remove meta for file '%s'!", subPathOrig.c_str());
						// Read the removed tag.
						std::string removedTagFile = dirPathMeta + subPathFilename + PIPELINE_DATABASE_REMOVE_SUFFIX;
						CFileRemove fr;
						CIFile ifr(removedTagFile, false);
						fr.serial(ifr);
						ifr.close();

						fileRemoveMap[subPathOrig] = fr;
					}
					
					// TODO_PROCESS_ERROR_EXIT
					if (g_IsExiting)
						return false;
				}
			}
			else
			{
				CFileStatus fs;
				if (!getFileStatus(fs, path))
				{
					// bad status, data corruption // TODO_PROCESS_ERROR
					nlwarning("Invalid status for '%s'!", path.c_str());
					return false; 
				}
				fileStatusMap[path] = fs; // i'll assume macropath might be easiest
			}
		}
		else
		{
			// TODO_PROCESS_WARNING
			nlwarning("Requesting status on file or directory '%s' that does not exist!", path.c_str());
			CFileRemove fr;
			std::string removedTagFile = getMetaFilePath(path, PIPELINE_DATABASE_REMOVE_SUFFIX);
			if (CFile::isExists(removedTagFile))
			{
				// file existed before
				CIFile ifr(removedTagFile, false);
				fr.serial(ifr);
				ifr.close();
			}
			else
			{
				// file never existed or is directory
				fr.Lost = 0;
			}
			fileRemoveMap[path] = fr;
		}
		// TODO_PROCESS_ERROR_EXIT
		if (g_IsExiting)
			return false;
	}
	
	// no issues occured apparently
	return true;
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
			std::string statusPath = getStatusFilePath(FilePath); // g_WorkspaceDirectory + PIPELINE_DATABASE_STATUS_SUBDIR + dropDatabaseDirectory(FilePath) + ".status";
			StatusMutex->lock_shared();
			bool statusFileExists = CFile::fileExists(statusPath);
			if (statusFileExists)
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
				// nlinfo("Skipping already updated status, may have been queued twice (%s)", FilePath.c_str());
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
				
				std::string removePath = getMetaFilePath(FilePath, PIPELINE_DATABASE_REMOVE_SUFFIX);

				StatusMutex->lock();
				{
					COFile ofs(statusPath, false, false, true);
					fs.serial(ofs);
					ofs.flush();
					ofs.close();
				}
				{
					// Important that we remove the remove after creating the status in case the service is killed inbetween.
					if (CFile::fileExists(removePath))
					{
						nlwarning("File '%s' was removed before, and now it exists again!", FilePath.c_str());
						CFile::deleteFile(removePath);
						nlinfo("Deleted '%s'", removePath.c_str());
						if (statusFileExists)
						{
							nlwarning("The remove file was not deleted last time the status file was created, this is either a bug, a sign of data corruption or it means the service was killed inbetween!");
						}
					}
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
	/*if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
		return NULL;
	}*/

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
	if (!CFile::isExists(subPath))
	{
		nlwarning("Updating status on path '%s' that does not exist!", subPath.c_str());
		return;
	}
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
	if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
	}

	std::string dirPath = standardizePath(dir, true);
	std::vector<std::string> dirContents;

	CPath::getPathContent(dirPath, false, true, true, dirContents);
	
	for (std::vector<std::string>::iterator it = dirContents.begin(), end = dirContents.end(); it != end; ++it)
	{
		const std::string &subPath = *it;
		
		updatePathStatus(ds, updater, subPath, recurse, false);
		
		if (g_IsExiting)
			return;
	}

	// REMOVE .STATUS METADATA OF REMOVED FILES -->
	// Not sure if this is a good location for this code, but works.

	// Removes .status metadata of files that no longer exists whenever the update of a directory is requested.
	// This sanely allows the service to know if someone deleted and then re-added the same file inbetween failed builds.
	// The FirstSeen field will be more recent than the timestamp of the last successful build start time.
	// Sanely get the process build start time by creating a dummy file at the beginning of the build and using that timestamp.
	// This file also unrelatedly can let us know if the service crashed or was killed during a build.

	// Note that we won't notice when a directory is deleted, this is also not necessary because we never work recursively in the service.
	// Any recursive lookups are done in specific unsafe build tools that are tagged as unsafe and not monitored by the pipeline.
	
	std::string dirPathMeta = getMetaFilePath(dirPath, "");
	// nldebug("META DIR: %s", dirPathMeta.c_str());
	std::vector<std::string> dirContentsMeta;

	CPath::getPathContent(dirPathMeta, false, false, true, dirContentsMeta);

	// This code is not guaranteed to work reliably.
	for (std::vector<std::string>::iterator it = dirContentsMeta.begin(), end = dirContentsMeta.end(); it != end; ++it)
	{
		const std::string &subPath = *it;
		
		std::string::size_type statusPos = subPath.find(PIPELINE_DATABASE_STATUS_SUFFIX);

		if (statusPos != std::string::npos)
		{
			std::string subPathFilename = CFile::getFilename(subPath.substr(0, statusPos));
			std::string subPathOrig = dirPath + subPathFilename;
			if (std::find(dirContents.begin(), dirContents.end(), subPathOrig) == dirContents.end())
			{
				// File no longer exists, warn the user.
				nlwarning("The file '%s' no longer exists!", subPathOrig.c_str());
				
				// Create the removed tag.
				std::string removedTagFile = dirPathMeta + subPathFilename + PIPELINE_DATABASE_REMOVE_SUFFIX;
				uint32 time = CTime::getSecondsSince1970();
				CFileRemove removed;
				removed.Lost = time; // Yes you're wasting time, you wouldn't have to delete anything if you did your work properly in the first place! :)
				COFile of(removedTagFile, false, false, true);
				removed.serial(of);
				of.flush();
				of.close();
				nldebug("Created '%s'", removedTagFile.c_str());
				
				// Delete the status.
				NLMISC::CFile::deleteFile(subPath);
				nlinfo("Removed '%s'", subPath.c_str());
			}
		}
		/*else
		{
			// TEMP
			// nldebug("Invalid file %s", subPath.c_str());
		}*/
		
		if (g_IsExiting)
			return;
	}

	// <-- END REMOVE
}

} /* anonymous namespace */

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback)
{
	/*if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
	}*/

	std::vector<std::string> paths;
	//paths.push_back(g_DatabaseDirectory);
	CConfigFile::CVar &rootDirectories = NLNET::IService::getInstance()->ConfigFile.getVar("RootDirectories");
	for (uint i = 0; i < rootDirectories.size(); ++i)
	{
		std::string rootDirectoryName = rootDirectories.asString(i);
		CConfigFile::CVar &dir = NLNET::IService::getInstance()->ConfigFile.getVar(rootDirectoryName);
		std::string rootDirectoryPath = standardizePath(dir.asString(), true);
		paths.push_back(rootDirectoryPath);
	}
	updateDatabaseStatus(callback, paths, false, true);
}

void CDatabaseStatus::updateDatabaseStatus(const CCallback<void> &callback, const std::vector<std::string> &paths, bool wait, bool recurse)
{
	/*if (!g_IsMaster)
	{
		nlerror("Not master, not allowed.");
	}*/

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
