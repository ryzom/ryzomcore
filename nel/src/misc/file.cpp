// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
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

#ifndef NL_OS_WINDOWS
#include <errno.h>
#endif

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
	_F = NULL;
	_Cache = NULL;
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
	_F=NULL;
	_Cache = NULL;
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
		_F = NULL;
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
				_F = fopen (filePath.c_str(), mode);
				if (_F != NULL)
				{
					_FileSize=CFile::getFileSize(_F);
					if (_FileSize == 0)
					{
						nlwarning("FILE: Size of file '%s' is 0", path.c_str());
						fclose(_F);
						_F = NULL;
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
		if(_F != NULL)
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
					_F = NULL;
				}
				return (_Cache != NULL);
			}
		}
	}

	// not in bnp, but may have '@' in the name
	if (_F == NULL)
	{
		_IsInBigFile = false;
		_IsInXMLPackFile = false;
		_BigFileOffset = 0;
		_AlwaysOpened = false;
		_F = nlfopen (path, mode);
		if (_F != NULL)
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
				_F = NULL;
			}
		}
		else
		{
			nlwarning("Failed to open file '%s', error %u : %s", path.c_str(), errno, strerror(errno));
			_FileSize = 0;
		}

		if ((_CacheFileOnOpen) && (_F != NULL))
		{
			// load file in the cache
			loadIntoCache();

			fclose (_F);
			_F = NULL;
			return (_Cache != NULL);
		}
	}

	return (_F != NULL);
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
			_Cache = NULL;
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
					_F = NULL;
				}
			}
		}
		else
		{
			if (_F)
			{
				fclose (_F);
				_F = NULL;
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
	if ((_CacheFileOnOpen) && (_Cache == NULL))
		throw EFileNotOpened (_FileName);
	if ((!_CacheFileOnOpen) && (_F == NULL))
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
	if ((_CacheFileOnOpen) && (_Cache == NULL))
		return false;
	if ((!_CacheFileOnOpen) && (_F == NULL))
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
	_F=NULL;
}

// ======================================================================================================
COFile::COFile(const std::string &path, bool append, bool text, bool useTempFile) : IStream(false)
{
	_F=NULL;
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
	if (append && useTempFile && CFile::fileExists(_FileName))
	{
		// open fails if can't copy original content
		if (!CFile::copyFile(_TempFileName, _FileName))
			return false;
	}

	_F = nlfopen(fileToOpen, mode);

	return _F!=NULL;
}
// ======================================================================================================
void	COFile::close()
{
	internalClose(true);
}
// ======================================================================================================
void	COFile::internalClose(bool success)
{
	if(_F)
	{
		fclose(_F);

		// Temporary filename ?
		if (!_TempFileName.empty())
		{
			// Delete old
			if (success)
			{
				// Bug under windows, sometimes the file is not deleted
				uint retry = 1000;
				while (--retry)
				{
					if (CFile::fileExists(_FileName))
						CFile::deleteFile (_FileName);

					if (CFile::moveFile(_FileName, _TempFileName))
						break;
					nlSleep (0);
				}
				if (!retry)
					throw ERenameError (_FileName, _TempFileName);
			}
			else
				CFile::deleteFile (_TempFileName);
		}

		_F=NULL;
	}
	resetPtrTable();
}
// ======================================================================================================
void	COFile::flush()
{
	if(_F)
	{
		fflush(_F);
	}
}


// ======================================================================================================
void		COFile::serialBuffer(uint8 *buf, uint len)
{
	if(!_F)
		throw	EFileNotOpened(_FileName);
	if(fwrite(buf, len, 1, _F) != 1)
//	if(fwrite(buf, 1, len, _F) != len)
	{
		if (ferror(_F) && errno == 28 /*ENOSPC*/)
		{
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

		if (nlfseek64 (_F, offset, origin_c)!=0)
			return false;
		return true;
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


