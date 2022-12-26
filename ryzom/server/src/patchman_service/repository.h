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

#ifndef REPOSITORY_H
#define REPOSITORY_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/misc/sstring.h"
#include "nel/misc/md5.h"

#include "file_manager.h"


//-----------------------------------------------------------------------------
// utility routines
//-----------------------------------------------------------------------------

NLMISC::CSString getRepositoryIndexFileName(const NLMISC::CSString& repositoryName);
uint32 getFileVersion(const NLMISC::CSString& fileName);


//-----------------------------------------------------------------------------
// class CRepository
//-----------------------------------------------------------------------------

class CRepository
{
public:
	// local data types
	struct CFilesMapEntry
	{
		uint32 FileSize;
		uint32 FileTime;
		NLMISC::CHashKeyMD5 Checksum;

		CFilesMapEntry(): FileSize(0), FileTime(0) {}
		void set(uint32 fileSize,uint32 fileTime,const NLMISC::CHashKeyMD5& checksum)
		{
			FileSize= fileSize;
			FileTime= fileTime;
			Checksum= checksum;
		}
	};
	typedef std::map<NLMISC::CSString,CFilesMapEntry> TFiles;
	typedef TFiles::iterator iterator;
	typedef TFiles::const_iterator const_iterator;

public:
	CRepository();

	// setup the target directory and scan for files
	// <name> defines the name used for temporary files and index files
	// <directory> defines the root directory to be managed by the repository
	// <filespec> defines the file specs that should be checked. eg: "*" or "*.bat;*.btm"
	// <avoid> defines the vector of files that block the normal update behaviour eg "toto/tata/??_busy_*" or "abc/def;ghi/jkl"
	// - if any of the 'avoid' files exist then we assume that some process is modifying the repository and we should wait for it to finish
	bool init(const NLMISC::CSString& name,const NLMISC::CSString& directory);

	// rescan target directory looking for new files and files that have changed
	uint32 update();

	// create a stub entry for a given file
	// - this needs to be called before calling 'updateFile()' for the first time
	void addFileStub(NLMISC::CSString fileName);

	// update the entry for a given file
	// - if the file is new then addFileStub() needs to be called first
	void updateFile(NLMISC::CSString fileName);

	// write index file to disk
	// this is called automatically by init() and update() but needs to be called by hand after manual
	// manipulation of map entries
	void writeIndexFile();

	// display info about the repository (version number, directory names, etc)
	void display(NLMISC::CLog& log=*NLMISC::InfoLog) const;

	// accessors for the version number
	uint32 getVersion() const;
	void setVersion(uint32 version);

	// wrapping of the stl map container interface
	uint32 size() const;

	const CFilesMapEntry& operator[](const NLMISC::CSString&) const;

	iterator		find(const NLMISC::CSString&);
	const_iterator	find(const NLMISC::CSString&) const;

	iterator		begin();
	const_iterator	begin() const;

	iterator		end();
	const_iterator	end() const;

	void			fillShortList(PATCHMAN::TFileInfoVector	&files) const;

private:

	// private data
	NLMISC::CSString _Name;
	NLMISC::CSString _TargetDirectory;
	uint32 _Version;	// The version number for the patch set to synchronise
	TFiles _Files;
};


//-----------------------------------------------------------------------------
#endif
