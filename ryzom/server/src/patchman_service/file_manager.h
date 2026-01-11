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

#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// nel
#include "nel/misc/singleton.h"
#include "nel/misc/sstring.h"
#include "nel/misc/md5.h"
#include "nel/misc/smart_ptr.h"
#include "nel/net/module.h"


//-----------------------------------------------------------------------------
// namespace PATCHMAN
//-----------------------------------------------------------------------------

namespace PATCHMAN
{		

	//-----------------------------------------------------------------------------
	// forward class declarations
	//-----------------------------------------------------------------------------

	struct SFileInfo;
	class CFileSpec;
	class IFileRequestValidator;
	class IFileInfoUpdateListener;
	class CRepositoryDirectory;
	class CFileManager;


	//-----------------------------------------------------------------------------
	// struct SFileInfo
	//-----------------------------------------------------------------------------

	struct SFileInfo
	{
		NLMISC::CSString FileName;	// this name includes the directory name
		uint32 FileSize;
		uint32 FileTime;
		NLMISC::CHashKeyMD5 Checksum;

		SFileInfo(): FileSize(0), FileTime(0) {}

		void clear()
		{
			FileName.clear();
			FileSize=0;
			FileTime=0;
			Checksum.clear();
		}

		void serial(NLMISC::IStream& stream)
		{
			stream.serial(FileName);
			stream.serial(FileSize);
			stream.serial(FileTime);
			stream.serial(Checksum);
		}

		// a '<' operator for use in STL containers and algorithms
		bool operator<(const SFileInfo& other) const
		{
			// compare file names
			if (FileName<other.FileName) return true;
			if (other.FileName<FileName) return false;

			// file names match so compare time stamps
			if (FileTime<other.FileTime) return true;
			if (other.FileTime<FileTime) return false;

			// looks like a match so far so compare sizes
			if (FileSize<other.FileSize) return true;
			if (other.FileSize<FileSize) return false;

			// the entries are identical so compare checksums
			return Checksum < other.Checksum;
		}

		// Type used as parameter to updateFileInfo()
		enum TUpdateMethod { DONT_RECALCULATE, RECALCULATE_IF_CHANGED, FORCE_RECALCULATE };

		// Update the file info by looking at the given file path on the disk
		// the fileName parameter is simply a label example: updateFileInfo("foo/bar.txt","c:/rootdir/foo/bar.txt")
		// The update methods determine whether or not to (re)calculate MD5 checksum
		// - DONT_RECALCULATE		- never recalculate
		// - RECALCULATE_IF_CHANGED - recalculate if either the file time or file size have changed
		// - FORCE_RECALCULATE		- always recalculate
		//
		// returns true if the record is valid and unchanged, othrwise returns false
		//
		bool updateFileInfo(const NLMISC::CSString& fileName,const NLMISC::CSString& fullFileName, TUpdateMethod updateMethod=RECALCULATE_IF_CHANGED, IFileInfoUpdateListener* updateListener=NULL);
	};
	typedef std::vector<SFileInfo> TFileInfoVector;
	typedef std::map<NLMISC::CSString,SFileInfo> TFileInfoMap;


	//-----------------------------------------------------------------------------
	// class CFileSpec
	//-----------------------------------------------------------------------------
	// This class encapsulates a filespec description
	// The file name and path sections of the spec are treated separately
	// eg:
	//	toto/*tata - will match all files matching '*tata' in directory toto
	//	toto*/tata - will match files called 'tata' in directories 'toto/', 
	//				 'toto_to/', 'toto/to/', etc
	//-----------------------------------------------------------------------------

	class CFileSpec
	{
	public:
		// ctors
		CFileSpec();
		CFileSpec(const NLMISC::CSString& fileSpec);

		// test a complete match (filename and path)
		bool matches(const NLMISC::CSString& fullFileName) const;

		// test whether the given file name matches the filename part of the filespec
		// note - the supplied filename should already have been stripped of its path
		bool nameMatches(const NLMISC::CSString& fileName) const;

		// test whether the given path matches the path part of the filespec
		// note - the supplied path should not have an attached file name
		bool pathMatches(const NLMISC::CSString& path) const;

		// retrieve the filespec as a single string (for serialising, etc)
		NLMISC::CSString toString() const;

		// accessors
		const NLMISC::CSString& nameSpec() const;
		const NLMISC::CSString& pathSpec() const;
		bool isWild() const;
		bool nameIsWild() const;
		bool pathIsWild() const;

	private:
		NLMISC::CSString _NameSpec;
		NLMISC::CSString _PathSpec;
		bool _NameIsWild;			// true if _NameSpec contains wildcards	('*' or '?')
		bool _PathIsWild;			// true if _PathSpec contains wildcards ('*' or '?')
		bool _AcceptAllNames;
		bool _AcceptAllPaths;
	};


	//-----------------------------------------------------------------------------
	// class IFileRequestValidator
	//-----------------------------------------------------------------------------
	// a callback interface to be implemented by classes who want to 
	// validate getFile() and getFileInfo() requests
	//-----------------------------------------------------------------------------

	class IFileRequestValidator
	{
	public:
		// overloadable callback methods
		virtual bool cbValidateDownloadRequest(const NLNET::IModuleProxy *sender,const std::string &fileName) { return true; }
		virtual bool cbValidateFileInfoRequest(const NLNET::IModuleProxy *sender,const std::string &fileName) { return true; }
	};


	//-----------------------------------------------------------------------------
	// class IFileInfoUpdateListener
	//-----------------------------------------------------------------------------
	// a callback interface to be implemented by classes who want to 
	// receive update notification for finle info changes
	//-----------------------------------------------------------------------------

	class IFileInfoUpdateListener
	{
	public:
		// overloadable callback methods
		virtual void cbFileInfoRescanning(const NLMISC::CSString& fileName,uint32 fileSize) {}	// called at start of rescan operation
		virtual void cbFileInfoUpdate(const SFileInfo& fileInfo) {}								// called at end of rescan
		virtual void cbFileInfoErased(const NLMISC::CSString& fileName) {}						//
	};


	//-----------------------------------------------------------------------------
	// class CRepositoryDirectory
	//-----------------------------------------------------------------------------

	class CRepositoryDirectory: public NLMISC::CRefCount
	{
	public:
		// handy types
		typedef	std::map<NLMISC::CSString,TFileInfoMap> TDirectoryTree;

		// update methods
		void clear();
		void rescanFull(IFileInfoUpdateListener* updateListener);
		void rescanPartial(IFileInfoUpdateListener* updateListener);
		void updateFile(const NLMISC::CSString& fileName,SFileInfo::TUpdateMethod updateMethod, IFileInfoUpdateListener* updateListener);

		// query methods
		void getFileInfo(const NLMISC::CSString& fileSpec,TFileInfoVector& result,IFileRequestValidator* validator,const NLNET::IModuleProxy *sender=NULL) const;
		void getFile(const NLMISC::CSString& fileName,NLMISC::CSString& resultData,IFileRequestValidator* validator,const NLNET::IModuleProxy *sender=NULL) const;

		// accessors
		const NLMISC::CSString &getRootDirectory() const;

		// persistence methods
		bool readIndex();
		bool writeIndex() const;


	private:
		// ctor is private because only CFileManager singleton has the right to create us
		friend class CFileManager;

		// ctor and init
		CRepositoryDirectory(const NLMISC::CSString& path);

	private:
		// private methods
		void _rescanDirectory(const NLMISC::CSString& directoryName, bool recurse, IFileInfoUpdateListener* updateListener);

		// private data
		NLMISC::CSString _Root;				// The root directory from which we are working
		TDirectoryTree _DirectoryTree;		// The set of files that we know (note, directories are mapped directly here)
		NLMISC::CSString _LastRescan;		// The name of the directory that we last rescanned
		mutable bool _IndexFileIsUpToDate;	// Is the index file on disk up to date (or does it need to be rewritten)
	};
	typedef NLMISC::CSmartPtr<CRepositoryDirectory> TRepositoryDirectoryPtr;
	typedef NLMISC::CRefPtr<CRepositoryDirectory> TRepositoryDirectoryRefPtr;


	//-----------------------------------------------------------------------------
	// class CFileManager
	//-----------------------------------------------------------------------------

	class CFileManager: public NLMISC::CSingleton<CFileManager>
	{
	public:
		// --------------------------------------------------------
		// public interface

		// loading file data
		bool load(const NLMISC::CSString& fileName, uint32 startOffset, uint32 numBytes, NLMISC::CSString& result);

		// saving files
		bool save(const NLMISC::CSString& fileName, const NLMISC::CMemStream& data);

		// accessing basic info about files
		uint32 getFileSize(const NLMISC::CSString& fileName);

		// get hold of a smart pointer to a repository directory object, creating a new one if need be
		// or sharing an object that already exists
		TRepositoryDirectoryPtr getRepositoryDirectory(const NLMISC::CSString& path);
		

	private:
		// --------------------------------------------------------
		// private data for the file cache
		std::vector<char> _CacheBuffer;

		struct SCacheFileEntry
		{
			NLMISC::CSString FileName;
			uint32 StartOffset;
			uint32 FileSize;
			uint32 FileTime;
		};

		typedef std::list<SCacheFileEntry> TCacheFiles;
		TCacheFiles _CacheFiles;

		// --------------------------------------------------------
		// private data for managing the set of repository directories
		typedef std::map<NLMISC::CSString,TRepositoryDirectoryRefPtr> TRepositoryDirectories;
		TRepositoryDirectories _RepositoryDirectories;
	};

} // end of namespace

//-----------------------------------------------------------------------------
#endif
