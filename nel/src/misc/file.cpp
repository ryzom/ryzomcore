// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010-2019  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2014-2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#include "nel/misc/file.h"
#include "nel/misc/debug.h"
#include "nel/misc/big_file.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/sstring.h"
#include "nel/misc/xml_pack.h"
#include "nel/misc/streamed_package_manager.h"

#include <errno.h>

using namespace std;

#define NLMISC_DONE_FILE_OPENED 40

#ifdef DEBUG_NEW
	#define new DEBUG_NEW
#endif

namespace NLMISC
{

typedef std::list<uint64> TFileAccessTimes;					// list of times at which a given file is opened for reading
typedef CHashMap<std::string,TFileAccessTimes> TFileAccessLog;	// map from file name to read access times
typedef NLMISC::CSynchronized<TFileAccessLog> TSynchronizedFileAccessLog;

static TSynchronizedFileAccessLog IFileAccessLog("IFileAccessLog");
static bool IFileAccessLoggingEnabled= false;
static uint64 IFileAccessLogStartTime= 0;

uint32 CIFile::_NbBytesSerialized = 0;
uint32 CIFile::_NbBytesLoaded = 0;
uint32 CIFile::_ReadFromFile = 0;
uint32 CIFile::_ReadingFromFile = 0;
uint32 CIFile::_FileOpened = 0;
uint32 CIFile::_FileRead = 0;
CSynchronized<std::deque<std::string> > CIFile::_OpenedFiles("");

// ======================================================================================================
CIFile::CIFile() : IStream(true)
{
	_F = nullptr;
	_Cache = nullptr;
	_ReadPos = 0;
	_FileSize = 0;
	_BigFileOffset = 0;
	_IsInBigFile = false;
	_IsInXMLPackFile = false;
	_CacheFileOnOpen = false;
	_IsAsyncLoading = false;
	_AllowBNPCacheFileOnOpen= true;
}

// ======================================================================================================
CIFile::CIFile(const std::string &path, bool text) : IStream(true)
{
	_F = nullptr;
	_Cache = nullptr;
	_ReadPos = 0;
	_FileSize = 0;
	_BigFileOffset = 0;
	_IsInBigFile = false;
	_IsInXMLPackFile = false;
	_CacheFileOnOpen = false;
	_IsAsyncLoading = false;
	_AllowBNPCacheFileOnOpen= true;
	open(path, text);
}

// ======================================================================================================
CIFile::~CIFile()
{
	close();
}


// ======================================================================================================
void		CIFile::loadIntoCache()
{
	const uint32 READPACKETSIZE = 64 * 1024;
	const uint32 INTERPACKETSLEEP = 5;

	_Cache = new uint8[_FileSize];
	if(!_IsAsyncLoading)
	{
		_ReadingFromFile += _FileSize;
		int read = (int)fread (_Cache, _FileSize, 1, _F);
		_FileRead++;
		_ReadingFromFile -= _FileSize;
		_ReadFromFile += read * _FileSize;
	}
	else
	{
		uint	index= 0;
		while(index<_FileSize)
		{
			if( _NbBytesLoaded + (_FileSize-index) > READPACKETSIZE )
			{
				sint	n= READPACKETSIZE-_NbBytesLoaded;
				n= max(n, 1);
				_ReadingFromFile += n;
				int read = (int)fread (_Cache+index, n, 1, _F);
				_FileRead++;
				_ReadingFromFile -= n;
				_ReadFromFile += read * n;
				index+= n;

				nlSleep (INTERPACKETSLEEP);
				_NbBytesLoaded= 0;
			}
			else
			{
				uint	n= _FileSize-index;
				_ReadingFromFile += n;
				int read = (int)fread (_Cache+index, n, 1, _F);
				_FileRead++;
				_ReadingFromFile -= n;
				_ReadFromFile += read * n;
				_NbBytesLoaded+= n;
				index+= n;
			}
		}
	}
}


// ======================================================================================================
bool		CIFile::open(const std::string &path, bool text)
{
	// Log opened files
	{
		CSynchronized<deque<string> >::CAccessor fileOpened(&_OpenedFiles);
		fileOpened.value().push_front (path);
		if (fileOpened.value().size () > NLMISC_DONE_FILE_OPENED)
			fileOpened.value().resize (NLMISC_DONE_FILE_OPENED);
		_FileOpened++;
	}

	close();

	if ((_IsInBigFile || _IsInXMLPackFile) && path.find('@') == string::npos)
	{
		// CIFile can be reused to load file from bnp and from regular files.
		// Last open happened to be inside bnp and close() may not set _F to NULL.
		// Opening regular file will fail as _F points to bnp file.
		_F = nullptr;
	}

	// can't open empty filename
	if(path.empty ())
		return false;

	// IFile Access Log management
	if (IFileAccessLoggingEnabled)
	{
		// get the current time
		uint64 timeNow = NLMISC::CTime::getPerformanceTime();

		// get a handle for the container
		TSynchronizedFileAccessLog::CAccessor synchronizedFileAccesLog(&IFileAccessLog);
		TFileAccessLog& fileAccessLog= synchronizedFileAccesLog.value();

		// add the current time to the container entry for the given path (creating a new container entry if required)
		fileAccessLog[path].push_back(timeNow);
	}

	char mode[3];
	mode[0] = 'r';
	mode[1] = 'b'; // No more reading in text mode
	mode[2] = '\0';

	_FileName = path;
	_ReadPos = 0;

	// Bigfile or xml pack access requested ?
	string::size_type pos;
	if ((pos = path.find('@')) != string::npos)
	{
		// check for a double @ to identify XML pack file
		if (pos+1 < path.size() && path[pos+1] == '@')
		{
			// xml pack file
			_IsInXMLPackFile = true;

			if(_AllowBNPCacheFileOnOpen)
			{
				_F = CXMLPack::getInstance().getFile(path, _FileSize, _BigFileOffset, _CacheFileOnOpen, _AlwaysOpened);
			}
			else
			{
				bool	dummy;
				_F = CXMLPack::getInstance().getFile (path, _FileSize, _BigFileOffset, dummy, _AlwaysOpened);
			}
		}
		else if (pos > 3 && path[pos-3] == 's' && path[pos-2] == 'n' && path[pos-1] == 'p')
		{
			// nldebug("Opening a streamed package file");

			_IsInXMLPackFile = false;
			_IsInBigFile = false;
			_BigFileOffset = 0;
			_AlwaysOpened = false;
			std::string filePath;
			if (CStreamedPackageManager::getInstance().getFile (filePath, path.substr(pos+1)))
			{
				_F = nlfopen(filePath, mode);
				if (_F != nullptr)
				{
					_FileSize=CFile::getFileSize(_F);
					if (_FileSize == 0)
					{
						nlwarning("FILE: Size of file '%s' is 0", path.c_str());
						fclose(_F);
						_F = nullptr;
					}
				}
				else
				{
					nlwarning("Failed to open file '%s', error %u : %s", path.c_str(), errno, strerror(errno));
					_FileSize = 0;
				}
			}
			else
			{
				// TEMPORARY ERROR
				// nlerror("File '%s' not in streamed package", path.c_str());
			}
		}
		else
		{
			// bnp file
			_IsInBigFile = true;
			if(_AllowBNPCacheFileOnOpen)
			{
				_F = CBigFile::getInstance().getFile (path, _FileSize, _BigFileOffset, _CacheFileOnOpen, _AlwaysOpened);
			}
			else
			{
				bool	dummy;
				_F = CBigFile::getInstance().getFile (path, _FileSize, _BigFileOffset, dummy, _AlwaysOpened);
			}
		}
		if(_F != nullptr)
		{
			// Start to load the bigfile or xml file at the file offset.
			nlfseek64 (_F, _BigFileOffset, SEEK_SET);

			// Load into cache ?
			if (_CacheFileOnOpen)
			{
				// load file in the cache
				loadIntoCache();

				if (!_AlwaysOpened)
				{
					fclose (_F);
					_F = nullptr;
				}
				return (_Cache != nullptr);
			}
		}
	}

	// not in bnp, but may have '@' in the name
	if (_F == nullptr)
	{
		_IsInBigFile = false;
		_IsInXMLPackFile = false;
		_BigFileOffset = 0;
		_AlwaysOpened = false;
		_F = nlfopen (path, mode);
		if (_F != nullptr)
		{
			/*
			THIS CODE REPLACED BY SADGE BECAUSE SOMETIMES
			ftell() RETRUNS 0 FOR NO GOOD REASON - LEADING TO CLIENT CRASH

			nlfseek64 (_F, 0, SEEK_END);
			_FileSize = ftell(_F);
			nlfseek64 (_F, 0, SEEK_SET);
			nlassert(_FileSize==filelength(fileno(_F)));

			THE FOLLOWING WORKS BUT IS NOT PORTABLE
			_FileSize=filelength(fileno(_F));
			*/
			_FileSize=CFile::getFileSize (_F);
			if (_FileSize == 0)
			{
				nlwarning ("FILE: Size of file '%s' is 0", path.c_str());
				fclose (_F);
				_F = nullptr;
			}
		}
		else
		{
			nlwarning("Failed to open file '%s', error %u : %s", path.c_str(), errno, strerror(errno));
			_FileSize = 0;
		}

		if ((_CacheFileOnOpen) && (_F != nullptr))
		{
			// load file in the cache
			loadIntoCache();

			fclose (_F);
			_F = nullptr;
			return (_Cache != nullptr);
		}
	}

	return (_F != nullptr);
}

// ======================================================================================================
void		CIFile::setCacheFileOnOpen (bool newState)
{
	_CacheFileOnOpen = newState;
}

// ======================================================================================================
void		CIFile::setAsyncLoading (bool newState)
{
	_IsAsyncLoading = true;
}


// ======================================================================================================
void		CIFile::close()
{
	if (_CacheFileOnOpen)
	{
		if (_Cache)
		{
			delete[] _Cache;
			_Cache = nullptr;
		}
	}
	else
	{
		if (_IsInBigFile || _IsInXMLPackFile)
		{
			if (!_AlwaysOpened)
			{
				if (_F)
				{
					fclose (_F);
					_F = nullptr;
				}
			}
		}
		else
		{
			if (_F)
			{
				fclose (_F);
				_F = nullptr;
			}
		}
	}
	nlassert(_Cache == NULL);
	resetPtrTable();
}

// ======================================================================================================
void		CIFile::flush()
{
	if (_CacheFileOnOpen)
	{
	}
	else
	{
		if (_F)
		{
			fflush (_F);
		}
	}
}

// ======================================================================================================
bool	CIFile::readAll(std::string &buffer)
{
	try
	{
		uint32 remaining = _FileSize;

		buffer.clear();
		buffer.reserve(_FileSize);
		while(!eof() && remaining > 0)
		{
			const static uint bufsize = 1024;
			char buf[bufsize];
			uint32 readnow = bufsize;
			if (readnow > remaining)
				readnow = remaining;

			serialBuffer((uint8 *)&buf[0], readnow);
			buffer.append(buf, readnow);
			remaining -= readnow;
		}
	}
	catch (const EFile &)
	{
		// buffer state is unknown
		return false;
	}

	return true;
}

// ======================================================================================================
void		CIFile::getline (char *buffer, uint32 bufferSize)
{
	if (bufferSize == 0)
		return;

	uint read = 0;
	for(;;)
	{
		if (read == bufferSize - 1)
		{
			*buffer = '\0';
			return;
		}

		try
		{
			// read one byte
			serialBuffer ((uint8 *)buffer, 1);
		}
		catch (const EFile &)
		{
			*buffer = '\0';
			return;
		}

		if (*buffer == '\n')
		{
			*buffer = '\0';
			return;
		}

		// skip '\r' char
		if (*buffer != '\r')
		{
			buffer++;
			read++;
		}
	}

}


// ======================================================================================================
bool		CIFile::eof ()
{
	return _ReadPos >= (sint32)_FileSize;
}

// ======================================================================================================
void		CIFile::serialBuffer(uint8 *buf, uint len)
{
	if (len == 0)
		return;
	// Check the read pos
	if ((_ReadPos < 0) || ((_ReadPos+len) > _FileSize))
		throw EReadError (_FileName);
	if ((_CacheFileOnOpen) && (_Cache == nullptr))
		throw EFileNotOpened (_FileName);
	if ((!_CacheFileOnOpen) && (_F == nullptr))
		throw EFileNotOpened (_FileName);

	if (_IsAsyncLoading)
	{
		_NbBytesSerialized += len;
		if (_NbBytesSerialized > 64 * 1024)
		{
			// give up time slice
			nlSleep (0);
			_NbBytesSerialized = 0;
		}
	}

	if (_CacheFileOnOpen)
	{
		memcpy (buf, _Cache + _ReadPos, len);
		_ReadPos += len;
	}
	else
	{
		int read;
		_ReadingFromFile += len;
		read=(int)fread(buf, len, 1, _F);
		_FileRead++;
		_ReadingFromFile -= len;
		_ReadFromFile += /*read **/ len;
		if (read != 1 /*< (int)len*/)
			throw EReadError(_FileName);
		_ReadPos += len;
	}
}

// ======================================================================================================
void		CIFile::serialBit(bool &bit)
{
	// Simple for now.
	uint8	v=bit;
	serialBuffer(&v, 1);
	bit=(v!=0);
}

// ======================================================================================================
bool		CIFile::seek (sint32 offset, IStream::TSeekOrigin origin) const
{
	if ((_CacheFileOnOpen) && (_Cache == nullptr))
		return false;
	if ((!_CacheFileOnOpen) && (_F == nullptr))
		return false;

	switch (origin)
	{
		case IStream::begin:
			_ReadPos = offset;
		break;
		case IStream::current:
			_ReadPos = _ReadPos + offset;
		break;
		case IStream::end:
			_ReadPos = _FileSize + offset;
		break;
		default:
			nlstop;
	}

	if (_CacheFileOnOpen)
		return true;

	// seek in the file. NB: if not in bigfile, _BigFileOffset==0.
	if (nlfseek64(_F, (sint64)_BigFileOffset + _ReadPos, SEEK_SET) != 0)
		return false;
	return true;
}

// ======================================================================================================
sint32		CIFile::getPos () const
{
	return _ReadPos;
}


// ======================================================================================================
std::string	CIFile::getStreamName() const
{
	return _FileName;
}


// ======================================================================================================
void	CIFile::allowBNPCacheFileOnOpen(bool newState)
{
	_AllowBNPCacheFileOnOpen= newState;
}


// ======================================================================================================
void	CIFile::dump (std::vector<std::string> &result)
{
	CSynchronized<deque<string> >::CAccessor acces(&_OpenedFiles);

	const deque<string> &openedFile = acces.value();

	// Resize the destination array
	result.clear ();
	result.reserve (openedFile.size ());

	// Add the waiting strings
	deque<string>::const_reverse_iterator ite = openedFile.rbegin ();
	while (ite != openedFile.rend ())
	{
		result.push_back (*ite);

		// Next task
		ite++;
	}
}

// ======================================================================================================
void	CIFile::clearDump ()
{
	CSynchronized<deque<string> >::CAccessor acces(&_OpenedFiles);
	acces.value().clear();
}

// ======================================================================================================
uint	CIFile::getDbgStreamSize() const
{
	return getFileSize();
}


// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
COFile::COFile() : IStream(false)
{
	_F = nullptr;
	m_WriteFailed = false;
	m_WriteErrno = 0;
}

// ======================================================================================================
COFile::COFile(const std::string &path, bool append, bool text, bool useTempFile) : IStream(false)
{
	_F = nullptr;
	m_WriteFailed = false;
	m_WriteErrno = 0;
	open(path, append, text, useTempFile);
}

// ======================================================================================================
COFile::~COFile()
{
	internalClose(false);
}
// ======================================================================================================
bool	COFile::open(const std::string &path, bool append, bool text, bool useTempFile)
{
	close();

	// can't open empty filename
	if(path.empty ())
		return false;

	_FileName = path;
	_TempFileName.clear();
	m_WriteFailed = false;
	m_WriteErrno = 0;

	char mode[3];
	mode[0] = (append)?'a':'w';
// ACE: NEVER SAVE IN TEXT MODE!!!	mode[1] = (text)?'\0':'b';
	mode[1] = 'b';
	mode[2] = '\0';

	string fileToOpen = path;
	if (useTempFile)
	{
		CFile::getTemporaryOutputFilename (path, _TempFileName);
		fileToOpen = _TempFileName;
	}

	// if appending to file and using a temporary file, copycat temporary file from original...
	bool copied = false;
	if (append && useTempFile && CFile::fileExists(_FileName))
	{
		// open fails if can't copy original content
		copied = CFile::copyFile(_TempFileName, _FileName);
		if (!copied)
		{
			// a part of the copy is not left behind
			if (CFile::isExists(_TempFileName))
				CFile::deleteFile(_TempFileName);
			_TempFileName.clear();
			return false;
		}
	}

	_F = nlfopen(fileToOpen, mode);
	if (_F == nullptr && useTempFile)
	{
		// nor is the copy when it cannot be opened
		if (copied)
			CFile::deleteFile(_TempFileName);
		_TempFileName.clear();
	}

	return _F != nullptr;
}
// ======================================================================================================
bool COFile::close()
{
	return internalClose(true);
}
// ======================================================================================================
// ENOSPC, and the quota of a user where the system has one: there is no room for the data
static bool isDiskFullErrno(int error)
{
#ifdef EDQUOT
	if (error == EDQUOT)
		return true;
#endif
	return error == ENOSPC;
}
// ======================================================================================================
void COFile::rememberWriteError(int error) const
{
	// the first failure is the one that tells why
	if (!m_WriteFailed)
	{
		m_WriteFailed = true;
		m_WriteErrno = error;
	}
}
// ======================================================================================================
bool COFile::internalClose(bool success)
{
	resetPtrTable();
	bool written = true;
	if(_F)
	{
		// What was written reaches the disk here at the latest: a small file is still entirely in the buffer
		// of the stream, so a full disk first shows when the close fails. A failed close is a failed write,
		// and so is an earlier write, flush or seek that failed: after a failed flush the close itself
		// succeeds, so those are remembered in the object and in the error flag of the stream.
		bool writeFailed = m_WriteFailed || ferror(_F) != 0;
		int writeErrno = m_WriteErrno;
		if (fclose(_F) != 0)
		{
			if (writeErrno == 0)
				writeErrno = errno;
			writeFailed = true;
		}

		// closed, whatever follows: the temporary file is dealt with below, and the destructor has nothing
		// left to close, to move or to delete
		_F = nullptr;
		m_WriteFailed = false;
		m_WriteErrno = 0;
		std::string tempFileName;
		tempFileName.swap(_TempFileName);

		if (tempFileName.empty())
		{
			// A plain file: nothing can be kept from being lost here, and close() is called from destructors
			// and from code that expects no exception of it. close() and the destructor log the failure,
			// and close() returns it to the callers that ask.
			if (writeFailed)
			{
				nlwarning("FILE: The file '%s' is not written completely, a write to it or its close failed, error %d : %s",
				    _FileName.c_str(), writeErrno, writeErrno != 0 ? strerror(writeErrno) : "no error code was kept");
				written = false;
			}
		}
		else if (!success || writeFailed)
		{
			// given up or incomplete: the temporary file goes, the file is not touched
			CFile::deleteFile(tempFileName);
			// The caller asked for the file to be replaced safely and has to know that it was not.
			// The destructor does not throw: it gives the writing up, which is what happened.
			if (success)
			{
				if (isDiskFullErrno(writeErrno))
					throw EDiskFullError(_FileName);
				throw EWriteError(_FileName);
			}
		}
		else
		{
			// The file is replaced in one step and is never deleted first. If it cannot be replaced it is
			// left as it was, and the temporary file is kept: it is the only copy of what was written.
			if (!CFile::replaceFile(_FileName, tempFileName))
				throw ERenameError(_FileName, tempFileName);
		}
	}
	return written;
}
// ======================================================================================================
void	COFile::flush()
{
	if(_F)
	{
		// nothing tells the caller that a flush failed, and the close after it succeeds: it is remembered
		if (fflush(_F) != 0)
			rememberWriteError(errno);
	}
}


// ======================================================================================================
void		COFile::serialBuffer(uint8 *buf, uint len)
{
	if(!_F)
		throw	EFileNotOpened(_FileName);
	// nothing to write, as in CIFile::serialBuffer: fwrite returns 0 for it, which is no failed write
	if (len == 0)
		return;
	if(fwrite(buf, len, 1, _F) != 1)
//	if(fwrite(buf, 1, len, _F) != len)
	{
		const int writeErrno = errno;
		if (ferror(_F))
		{
			rememberWriteError(writeErrno);
			if (isDiskFullErrno(writeErrno))
				throw EDiskFullError(_FileName);
		}
		throw	EWriteError(_FileName);
	}
}
// ======================================================================================================
void		COFile::serialBit(bool &bit)
{
	// Simple for now.
	uint8	v=bit;
	serialBuffer(&v, 1);
}
// ======================================================================================================
bool		COFile::seek (sint32 offset, IStream::TSeekOrigin origin) const
{
	if (_F)
	{
		int origin_c = SEEK_SET;
		switch (origin)
		{
		case IStream::begin:
			origin_c=SEEK_SET;
			break;
		case IStream::current:
			origin_c=SEEK_CUR;
			break;
		case IStream::end:
			origin_c=SEEK_END;
			break;
		default:
			nlstop;
		}

		// A seek first writes what the stream has buffered. When that write fails the data is lost, and the
		// callers that seek back to fill in a size do not look at what the seek returns: the error flag of
		// the stream tells, and close() reports it.
		const bool seekFailed = nlfseek64(_F, offset, origin_c) != 0;
		const int seekErrno = errno;
		if (ferror(_F))
			rememberWriteError(seekErrno);
		return !seekFailed;
	}
	return false;
}
// ======================================================================================================
sint32		COFile::getPos () const
{
	if (_F)
	{
		return ftell (_F);
	}
	return 0;
}

// ======================================================================================================
std::string		COFile::getStreamName() const
{
	return _FileName;
}



// ======================================================================================================
// ======================================================================================================
// ======================================================================================================


// ======================================================================================================
NLMISC_CATEGORISED_COMMAND(nel, iFileAccessLogStart, "Start file access logging", "")
{
	if (!args.empty())
		return false;

	IFileAccessLoggingEnabled= true;
	if (IFileAccessLogStartTime==0)
	{
		uint64 timeNow = NLMISC::CTime::getPerformanceTime();
		IFileAccessLogStartTime= timeNow;
	}

	return true;
}

// ======================================================================================================
NLMISC_CATEGORISED_COMMAND(nel, iFileAccessLogStop, "Stop file access logging", "")
{
	if (!args.empty())
		return false;

	IFileAccessLoggingEnabled= false;

	return true;
}

// ======================================================================================================
NLMISC_CATEGORISED_COMMAND(nel, iFileAccessLogClear, "Clear file access logs", "")
{
	if (!args.empty())
		return false;

	TSynchronizedFileAccessLog::CAccessor(&IFileAccessLog).value().clear();

	return true;
}

// ======================================================================================================
NLMISC_CATEGORISED_COMMAND(nel, iFileAccessLogDisplay, "Display file access logs", "")
{
	if (!args.empty())
		return false;

	log.displayNL("-- FILE ACCESS LOG BEGIN --");

	TSynchronizedFileAccessLog::CAccessor fileAccesLog(&IFileAccessLog);
	TFileAccessLog::const_iterator it= fileAccesLog.value().begin();
	TFileAccessLog::const_iterator itEnd= fileAccesLog.value().end();
	uint32 count=0;
	while (it!=itEnd)
	{
		uint32 numTimes= (uint32)it->second.size();
		CSString fileName= it->first;
		if (fileName.contains("@"))
		{
			log.display("%d,%s,%s,",numTimes,fileName.splitTo("@").c_str(),fileName.splitFrom("@").c_str());
		}
		else
		{
			log.display("%d,,%s,",numTimes,fileName.c_str());
		}
		TFileAccessTimes::const_iterator atIt= it->second.begin();
		TFileAccessTimes::const_iterator atItEnd=it->second.end();
		while (atIt!=atItEnd)
		{
			uint64 delta= (*atIt-IFileAccessLogStartTime);
			log.display("%" NL_I64 "u,",delta);
			++atIt;
		}
		log.displayNL("");
		++count;
		++it;
	}

	log.displayNL("-- FILE ACCESS LOG END (%d Unique Files Accessed) --",count);

	return true;
}

}


