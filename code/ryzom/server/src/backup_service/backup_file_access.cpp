// Ryzom - MMORPG Framework <http://dev.ryzom.com/projects/ryzom/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#include "backup_file_access.h"

#include "nel/misc/path.h"
#include "nel/misc/mem_stream.h"
#include "nel/misc/file.h"
#include "nel/net/service.h"

#include "game_share/backup_service_messages.h"
#include "server_share/backup_service_itf.h"

#include "backup_service.h"

using namespace NLNET;


/* These variables are deprecated. Now the BS only deals with paths relative to SaveShardRoot.
NLMISC::CVariable<std::string>	BSFilePrefix("backup", "BSFilePrefix", "file read/write prefix", "", 0, true);
NLMISC::CVariable<std::string>	BSFileSubst("backup", "BSFileSubst", "file read/write substitute part", "", 0, true);
*/
NLMISC::CVariable<bool>	VerboseLog("backup", "VerboseLog", "Activate verbose logging of BS activity", false);
NLMISC::CVariable<bool>	UseTempFile("backup", "UseTempFile", "Flag the use of temporary file for safe write or append operation", true, true);

extern NLMISC::CVariable<std::string> SaveShardRoot;

bool	bsstrincmp(const char* s1, const char* s2, int n)
{
	int	nn;
	for (nn = (int)n; nn>0 && *s1 != '\0' && *s2 != '\0' && tolower(*s1) == tolower(*s2); --nn, ++s1, ++s2)
		;
	return nn == 0 || (*s1 == *s2);
}

std::string	getBackupFileName(const std::string& filename)
{
	return SaveShardRoot.get() + filename;
	/* // BSFilePrefix and BSFileSubst are deprecated
	if (BSFilePrefix.get().empty())
		return filename;

	if (!BSFileSubst.get().empty() && bsstrincmp(BSFileSubst.get().c_str(), filename.c_str(), BSFileSubst.get().size()))
		return BSFilePrefix.get() + filename.substr(BSFileSubst.get().size());
	else
		return BSFilePrefix.get() + filename;
	*/
}


// Init File manager
void	CFileAccessManager::init()
{
	_Mode = Normal;
	_StallAllowed = true;
	_Accesses.clear();
}

// Add an access to perform to stack of accesses
void	CFileAccessManager::stackFileAccess(IFileAccess* access)
{
	_Accesses.push_back(access);
}

// Flushes file accesses
void	CFileAccessManager::update()
{
	// while we are in normal mode, and there are still accesses to perform
	while (_Mode == Normal && !_Accesses.empty())
	{
		// get next access
		IFileAccess*				access = _Accesses.front();
		IFileAccess::TReturnCode	rc = access->execute(*this);

		if (rc == IFileAccess::MajorFailure)
		{
			nlwarning("Failed to execute access to file '%s', setting to STALLED mode", access->Filename.c_str());
			setMode(Stalled, access->FailureReason);
		}
		else
		{
			if (rc == IFileAccess::MinorFailure)
			{
				nlwarning("Minor failure in access to file '%s', access is discarded yet.", access->Filename.c_str());
			}

			_Accesses.pop_front();
			delete access;
		}
	}
}

// Release File manager
void	CFileAccessManager::release()
{
	if (!_Accesses.empty())
		update();

	if (!_Accesses.empty())
	{
		nlwarning("WARNING: release asked as file manager still contains pending accesses!");
		nlwarning("DATA WILL BE LOST!!");
	}
}



// Force manager mode
void	CFileAccessManager::setMode(TMode mode, const std::string& reason)
{
	if (mode == _Mode)
		return;

	_Mode = mode;
	_Reason = reason;

	NLNET::CMessage	msgout("STALL_MODE");
	bool			stalled = (_Mode != Normal);
	std::string		reasonstr = _Reason;

	msgout.serial(stalled, reasonstr);
	NLNET::CUnifiedNetwork::getInstance()->send("EGS", msgout);

	if (_Mode == Stalled)
		IService::getInstance()->setCurrentStatus("Stalled");
	else
		IService::getInstance()->clearCurrentStatus("Stalled");

}



// Notify service connection
void	CFileAccessManager::notifyServiceConnection(NLNET::TServiceId serviceId, const std::string& serviceName)
{
	if (serviceName == "EGS" && _Mode == Stalled)
	{
		NLNET::CMessage	msgout("STALL_MODE");
		bool			stalled = true;
		std::string		reason = _Reason;

		msgout.serial(stalled, reason);
		NLNET::CUnifiedNetwork::getInstance()->send(serviceId, msgout);
	}
}



// display stacked accesses
void	CFileAccessManager::displayFileAccesses(NLMISC::CLog& log)
{
	uint	i;
	for (i=0; i<_Accesses.size(); ++i)
	{
		log.displayRawNL("%2d %08p %s %s %s", i, _Accesses[i], _Accesses[i]->Requester.toString().c_str(), _Accesses[i]->Filename.c_str(), _Accesses[i]->FailureReason.c_str());
	}

}

// Remove a file access
void	CFileAccessManager::removeFileAccess(IFileAccess* access)
{
	std::deque<IFileAccess*>::iterator	it;
	for (it=_Accesses.begin(); it!=_Accesses.end(); ++it)
	{
		if ((*it) == access)
		{
			delete (*it);
			_Accesses.erase(it);
			return;
		}
	}
}



IFileAccess::TReturnCode	CLoadFile::execute(CFileAccessManager& manager)
{
	bool	fileExists = NLMISC::CFile::fileExists(getBackupFileName(Filename));

	if (!fileExists && checkFailureMode(MajorFailureIfFileNotExists))
	{
		FailureReason = NLMISC::toString("MAJOR_FAILURE:LOAD: file '%s' doesn't not exist", Filename.c_str());
		return MajorFailure;
	}

	CBackupMsgReceiveFile outMsg;
	outMsg.RequestId = RequestId;

	NLMISC::CIFile	f;
	bool			fileOpen = (fileExists && f.open(getBackupFileName(Filename)));
	bool			fileRead = false;

	if (fileOpen)
	{
		outMsg.FileDescription.set(getBackupFileName(Filename));

		// restore filename with the provided one for the response
		outMsg.FileDescription.FileName = Filename;

		try
		{
			H_AUTO(LoadFileSerial);
  			outMsg.Data.invert();
			f.serialBuffer( outMsg.Data.bufferToFill(outMsg.FileDescription.FileSize), outMsg.FileDescription.FileSize);
			outMsg.Data.invert();
			fileRead = true;
			f.close();
		}
		catch(const NLMISC::Exception &)
		{
		}
	}

	if (!fileRead && checkFailureMode(MajorFailureIfFileUnreaddable))
	{
		FailureReason = NLMISC::toString("MAJOR_FAILURE:LOAD: can't %s file '%s'", (!fileOpen ? "open" : "read"), Filename.c_str());
		return MajorFailure;
	}

//	outMsg.send(Requester);
	switch (Requester.RequesterType)
	{
	case TRequester::rt_service:
		{
			H_AUTO(CBackupMsgReceiveFile1);
			NLNET::CMessage msgOut("bs_file");
			outMsg.serial(msgOut);
			NLNET::CUnifiedNetwork::getInstance()->send( Requester.ServiceId, msgOut );
		}
		break;
	case TRequester::rt_layer3:
		{
			NLNET::CMessage msgOut("bs_file");
			outMsg.serial(msgOut);
			Requester.Netbase->send(msgOut, Requester.From);
		}
		break;
	case TRequester::rt_module:
		{
			BS::CBackupServiceClientProxy bsc(Requester.ModuleProxy);

			bsc.loadFileResult(CBackupService::getInstance()->getBackupModule(), 
				RequestId, 
				outMsg.FileDescription.FileName, 
				outMsg.FileDescription.FileTimeStamp, 
				NLNET::TBinBuffer(outMsg.Data.buffer(), outMsg.Data.length()));
		}
	}

	// If the file read failed for any other reason than file not found then complain
	if (!fileRead && fileExists)
	{
        FailureReason = NLMISC::toString("MINOR_FAILURE:LOAD: can't %s file '%s'", (!fileOpen ? "open" : "read"), Filename.c_str());
        return MinorFailure;
	}

	if (VerboseLog)
	{
		// We can assume that this is the only case where the file read failed that hasn't already been treated above
		if (!fileExists)
		{
			nldebug("Load file Failed (but MajorFailureIfFileUnreaddable==false): file '%s' doesn't not exist", Filename.c_str());
		}
		else
		{
			nlinfo("Loaded file '%s'", Filename.c_str());
		}
	}

	return Success;
}



CWriteFile::CWriteFile(const std::string& filename, const TRequester &requester, uint32 requestid, NLMISC::CMemStream& data)
	: IFileAccess(filename, requester, requestid)
{
	BackupFile = false;
	Append = false;
	CreateDir = true;
	FailureMode = MajorFailureMode;
	uint32 startPos = (uint32)data.getPos();
	uint32 actualLen = data.length()-startPos;
	Data.resize(actualLen);
	memcpy(&(Data[0]), data.buffer()+startPos, actualLen);
}

CWriteFile::CWriteFile(const std::string& filename, const TRequester &requester, uint32 requestid, uint8* data, uint dataSize)
	: IFileAccess(filename, requester, requestid)
{
	BackupFile = false;
	Append = false;
	CreateDir = true;
	FailureMode = MajorFailureMode;
	Data.resize(dataSize);
	memcpy(&(Data[0]), data, dataSize);
}

IFileAccess::TReturnCode	CWriteFile::execute(CFileAccessManager& manager)
{
	bool	fileExists = NLMISC::CFile::fileExists(getBackupFileName(Filename));

	if (!fileExists)
	{
		if (checkFailureMode(MajorFailureIfFileNotExists))
		{
			FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: file '%s' does not exist", Filename.c_str());
			return MajorFailure;
		}
		if (checkFailureMode(MinorFailureIfFileNotExists))
		{
			FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: file '%s' does not exist", Filename.c_str());
			return MinorFailure;
		}
	}
	else
	{
		if (checkFailureMode(MajorFailureIfFileExists))
		{
			FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: file '%s' already exists", Filename.c_str());
			return MajorFailure;
		}
		if (checkFailureMode(MinorFailureIfFileExists))
		{
			FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: file '%s' already exists", Filename.c_str());
			return MinorFailure;
		}
	}

	if (CreateDir)
	{
		std::string	dir = NLMISC::CFile::getPath(getBackupFileName(Filename));

		if (!NLMISC::CFile::isExists(dir))
		{
			if (!NLMISC::CFile::createDirectoryTree(dir) ||
				!NLMISC::CFile::setRWAccess(dir))
			{
				if (checkFailureMode(MajorFailureIfFileUnwritable))
				{
					FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: can't open file '%s', failed to create directory", Filename.c_str());
					return MajorFailure;
				}
				FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: can't open file '%s', failed to create directory", Filename.c_str());
				return MinorFailure;
			}

			if (VerboseLog)
				nlinfo("Created directory tree '%s'", dir.c_str());
		}
	}

	// if can't open file, file is unwritable, failure in all cases (and no backup)
	// file is kept untouched
	NLMISC::COFile	f;
	bool	fileOpen = f.open(getBackupFileName(Filename), Append, false, UseTempFile);
	if (!fileOpen)
	{
		if (checkFailureMode(MajorFailureIfFileUnwritable))
		{
			FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: can't open file '%s'", Filename.c_str());
			return MajorFailure;
		}
		FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: can't open file '%s'", Filename.c_str());
		return MinorFailure;
	}

	bool	fileSaved = false;

	try
	{
		f.serialBuffer(&(Data[0]), (uint)Data.size());
		fileSaved = true;

		if (VerboseLog)
			nlinfo("%s %u octets to file '%s'", Append ? "Append" : "Save", Data.size(), Filename.c_str());
	}
	catch(const NLMISC::Exception &)
	{
	}

	// if can't write in file, file is unwritable, failure in all cases (and no backup)
	// file is kept untouched
	if (!fileSaved && checkFailureMode(MajorFailureIfFileUnwritable))
	{
		if (checkFailureMode(MajorFailureIfFileUnwritable))
		{
			FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: can't write file '%s'", Filename.c_str());
			return MajorFailure;
		}
		FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: can't write file '%s'", Filename.c_str());
		return MinorFailure;
	}

	// if backup failed
	//  -> if it is a major issue, don't write file
	//  -> else write file anyway
	bool	fileBackuped = true;
	if (fileExists && BackupFile)
	{
		if (!NLMISC::CFile::copyFile( getBackupFileName(Filename)+".backup", getBackupFileName(Filename)))
		{
			fileBackuped = false;
			if (checkFailureMode(MajorFailureIfFileUnbackupable))
			{
				FailureReason = NLMISC::toString("MAJOR_FAILURE:WRITE: can't backup file '%s'", Filename.c_str());
				return MajorFailure;
			}
		}
	}

	f.close();

	if (!fileBackuped)
	{
		FailureReason = NLMISC::toString("MINOR_FAILURE:WRITE: can't backup file '%s'", Filename.c_str());
		return MinorFailure;
	}

	NLMISC::COFile flog;
	std::string str = getBackupFileName(Filename)+"\n";
	if(str.find("characters")!=std::string::npos)
	{
		flog.open(getBackupFileName("new_save.txt"), true);
		flog.serialBuffer((uint8*)&(str[0]), str.size());
		flog.close();
	}

	return Success;
}


IFileAccess::TReturnCode	CDeleteFile::execute(CFileAccessManager& manager)
{
	bool	fileExists = NLMISC::CFile::fileExists(getBackupFileName(Filename));

	if (!fileExists)
	{
		if (checkFailureMode(MajorFailureIfFileNotExists))
		{
			FailureReason = NLMISC::toString("MAJOR_FAILURE:DELETE: file '%s' does not exist", Filename.c_str());
			return MajorFailure;
		}
		if (checkFailureMode(MinorFailureIfFileNotExists))
		{
			FailureReason = NLMISC::toString("MINOR_FAILURE:DELETE: file '%s' does not exist", Filename.c_str());
			return MinorFailure;
		}
		return Success;
	}

	if (BackupFile)
	{
		bool	fileBackuped = false;
		try
		{
			std::string	path = NLMISC::CFile::getPath(getBackupFileName(Filename));
			std::string	file = NLMISC::CFile::getFilename(Filename);
			std::string	backup;
			uint	i = 0;
			do
			{
				backup = NLMISC::toString("%sdelete.%04u.%s", path.c_str(), i++, file.c_str());
			}
			while (i <= 10000 && NLMISC::CFile::fileExists(backup));

			fileBackuped = (i <= 10000 && NLMISC::CFile::moveFile(backup.c_str(), (getBackupFileName(Filename)).c_str()));
		}
		catch (...)
		{
		}

		if (!fileBackuped)
		{
			if (checkFailureMode(MajorFailureIfFileUnbackupable))
			{
				FailureReason = NLMISC::toString("MAJOR_FAILURE:DELETE: can't backup file '%s'", Filename.c_str());
				return MajorFailure;
			}
			FailureReason = NLMISC::toString("MINOR_FAILURE:DELETE: can't backup file '%s'", Filename.c_str());
			return MinorFailure;
		}
	}
	else
	{
		if (!NLMISC::CFile::deleteFile(getBackupFileName(Filename)))
		{
			if (checkFailureMode(MajorFailureIfFileUnDeletable))
			{
				FailureReason = NLMISC::toString("MAJOR_FAILURE:DELETE: can't delete file '%s'", Filename.c_str());
				return MajorFailure;
			}
			FailureReason = NLMISC::toString("MINOR_FAILURE:DELETE: can't delete file '%s'", Filename.c_str());
			return MinorFailure;
		}
	}

	if (VerboseLog)
		nlinfo("Deleted file '%s'", Filename.c_str());

	return Success;
}


