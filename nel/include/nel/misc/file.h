// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
// Copyright (C) 2010  Winch Gate Property Limited
//
// This source file has been modified by the following contributors:
// Copyright (C) 2020  Jan BOON (Kaetemi) <jan.boon@kaetemi.be>
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

#ifndef NL_FILE_H
#define NL_FILE_H

#include "types_nl.h"
#include "stream.h"



namespace NLMISC
{

// ======================================================================================================
/**
 * File Exception.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
struct EFile : public EStream
{
	EFile () : EStream( "Unknown file error" ) {}
	EFile (const std::string& filename) : EStream( "Unknown file error in '"+filename+"'" ), Filename(filename) {}
	EFile (const std::string& filename, const std::string& text, bool ) : EStream( text ), Filename(filename) {}

	virtual ~EFile() NL_OVERRIDE {}

	std::string Filename;
};

struct EFileNotOpened : public EFile
{
	EFileNotOpened( const std::string& filename ) : EFile( filename, "File '"+filename+"' not opened", true ) {}
};

struct EReadError : public EFile
{
	EReadError( const std::string& filename ) : EFile( filename, "Read error in file '" +filename+"' (End of file?)", true ) {}
};

struct EWriteError : public EFile
{
	EWriteError( const std::string& filename ) : EFile( filename, "Write Error in file '" +filename+"'", true ) {}
};

struct EDiskFullError : public EWriteError
{
	EDiskFullError( const std::string& filename ) : EWriteError(filename) {}
};

struct ERenameError : public EFile
{
	ERenameError( const std::string& dest, const std::string& src ) : EFile( dest, "Rename Error from the file '" +src+"' to the file '" +dest+"'", true ) {}
};


// ======================================================================================================
/**
 * Input File.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 * ***********************************************
 *	WARNING: This Class/Method must be thread-safe (ctor/dtor/serial): no static access for instance
 *	It can be loaded/called through CAsyncFileManager for instance
 * ***********************************************
 */
class CIFile : public IStream
{
public:		// Basic Usage.
	/// Object. NB: destructor close() the stream.
	CIFile();
	CIFile(const std::string &path, bool text=false);
	~CIFile();

	/// Open a file for reading. false if failed. close() if a file was opened.
	bool	open (const std::string &path, bool text=false);

	/// Set the cache file on open option (default behaviour is false (file is not cached at opening)
	void	setCacheFileOnOpen (bool newState);

	/** If the file is opened with a big file, CacheFileOnOpen is replaced with big file option. Except if
	 *	allowBNPCacheFileOnOpen(false) is called. true is default.
	 */
	void	allowBNPCacheFileOnOpen(bool newState);

	/// Set the async loading state (to go to sleep 5 ms after 100 Ko serialized)
	void	setAsyncLoading (bool newState);

public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint32 offset, IStream::TSeekOrigin origin) const;
	/// Get the location of the file pointer
	sint32	getPos () const;

	// Imp the Name of the stream as the name of the file.
	virtual std::string		getStreamName() const;

	// same function that in ifstream
	// return a string separated by \n or eof, used to parsing text file
	void getline (char *buffer, uint32 bufferSize);

	// read whole file into a string. resulting buffer may contain NULL chars.
	// internal read position is modified.
	// return true on success, false on failure.
	bool readAll(std::string &buffer);

	// return the size of the file
	uint32 getFileSize () const { return _FileSize; }

	// return true if there's nothing more to read (same as ifstream)
	bool eof ();

	virtual void serialBuffer(uint8 *buf, uint len);

	/// \name Statistics

	/// Get the number of file open from the beginning of the application. Files can be in a big file.
	static uint32	getNumFileOpen() {return _FileOpened;}

	/// Get the number of read acces to a file.
	static uint32	getNumFileRead() {return _FileRead;}

	/// Get the number of byte read from the file system since the application start.
	static uint32	getReadFromFile() {return _ReadFromFile;}

	/// Get the number of byte being reading from the file system at the moment.
	static uint32	getReadingFromFile() {return _ReadingFromFile;}

	/// Get the last 40 files opened. The files can be in a big file.
	static void		dump (std::vector<std::string> &result);

	/// clear the dump of the last 40 files opened
	static void		clearDump ();

protected:
	virtual void		serialBit(bool &bit);

	virtual uint		getDbgStreamSize() const;


private:
	FILE		*_F;
	std::string _FileName;

	// Async
	static uint32 _NbBytesSerialized;
	static uint32 _NbBytesLoaded;

	// Stats
	static uint32 _FileOpened;
	static uint32 _FileRead;
	static uint32 _ReadFromFile;
	static uint32 _ReadingFromFile;
	static CSynchronized<std::deque<std::string> > _OpenedFiles;

	bool _IsAsyncLoading;

	// Cache
	bool			_CacheFileOnOpen;
	bool			_AllowBNPCacheFileOnOpen;
	uint8			*_Cache;
	mutable sint32	_ReadPos;
	uint32			_FileSize;

	// Big file & xml pack
	bool	_AlwaysOpened;
	/// Flag true if file is in a big file
	bool	_IsInBigFile;
	/// Flag true if file is in an xml pack
	bool	_IsInXMLPackFile;
	//// Offset in bnp or xml pack
	uint32	_BigFileOffset;

	// Load async if needed in the cache.
	void	loadIntoCache();
};


// ======================================================================================================
/**
 * Output File.
 * \author Lionel Berenguier
 * \author Nevrax France
 * \date 2000
 */
class COFile : public IStream
{
public:		// Basic Usage.
	/// Object. NB: destructor close() the stream.
	COFile();
	COFile(const std::string &path, bool append=false, bool text=false, bool useTempFile=false);
	~COFile();

	/** Open a file for writing. false if failed. close() if a file was opened.
	*	If you open the file with the flag useTempFile, you MUST close explicitly the file
	*	with close() if the writing as been successed.
	*/
	bool	open(const std::string &path, bool append=false, bool text=false, bool useTempFile=false);

	bool	isOpen	()	const
	{
		return	_F!=NULL;
	}

public:		// Advanced Usage.
	/// Explicit close.
	void	close();
	/// flush the file.
	void	flush();
	/// Seek the file
	bool	seek (sint32 offset, IStream::TSeekOrigin origin) const;
	/// Get the location of the file pointer
	sint32	getPos () const;

	// Imp the Name of the stream as the name of the file.
	virtual std::string		getStreamName() const;

	// very useful to serialize string in text mode (without the size)
	virtual void		serialBuffer(uint8 *buf, uint len);

protected:
	/// Internal close.
	void	internalClose(bool success);
	virtual void		serialBit(bool &bit);

private:
	FILE	*_F;
	std::string _FileName;
	std::string _TempFileName;
};


}


#endif // NL_FILE_H

/* End of file.h */
