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

#ifndef FILE_DESCRIPTION_CONTAINER_H
#define	FILE_DESCRIPTION_CONTAINER_H

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "nel/misc/types_nl.h"
#include "nel/misc/stream.h"
#include "nel/misc/sstring.h"

#include <vector>


//-----------------------------------------------------------------------------
// class CFileDescription
//-----------------------------------------------------------------------------

class CFileDescription
{
public:
	NLMISC::CSString FileName;
	uint32 FileTimeStamp;
	uint32 FileSize;

	// ctor
	CFileDescription(const std::string& name=std::string(),uint32 time=0,uint32 size=0);

	// setup the record for a given file
	// if file exists then initialises file size and time stamp accordingly and return true
	// else zeros file size and timestamp and returns false
	bool set(const std::string& name);

	// serialise...
	void serial(NLMISC::IStream& stream);

	// generate a complete string description of the file
	// the 'maxFileNameLen' is used to generate padding spaces to allign file lengths nicely
	NLMISC::CSString toString(uint32 maxFileNameLen=0) const;

	// remove the provided string if found at the beginning of FileName
	void stripFilename(const std::string& header) { removeHeaderFromFileName(header); }

	// comparison operator for use with STL
	bool operator<(const CFileDescription& other) const;

protected:
	// remove the provided string if found at the beginning of FileName
	void removeHeaderFromFileName( const std::string& header )
	{
		uint hdsize = (uint)header.size();
		if ( FileName.substr( 0, hdsize ) == header )
		{
			FileName = FileName.substr( hdsize );
		}
	}
};


//-----------------------------------------------------------------------------
// class CFileDescriptionContainer
//-----------------------------------------------------------------------------

class CFileDescriptionContainer
{
public:
	// add a specific named file to the container
	void addFile(const CFileDescription& fileDescription);

	// add a specific named file to the container
	void addFile(const std::string& fileName, uint32 timeStamp, uint32 size);

	// add a specific named file to the container
	// the file size and timestamp are looked up on the disk
	void addFile(const std::string& fileName);

	// add all files matching the given file spec to the container
	void addFileSpec(const std::string& fileSpec, bool Recurse=false);

	// add all files in a given directory that match any of the supplied wildcards to the container
	void addFiles(const std::string& directory, const std::vector<std::string>& wildcards, bool recurse=false);
	void addFiles(const std::string& directory, const NLMISC::CVectorSString& wildcards, bool recurse=false);

	// add the contents of another fdc to this one
	void addFiles(const CFileDescriptionContainer& other);

	// display a list of the files in the container to the named output log
	void display(NLMISC::CLog* log) const;

	// serialise...
	void serial(NLMISC::IStream& stream);

	// get number of elemnts in the container
	uint32 size() const;

	// check whether the container is empty
	bool empty() const;

	// clear out the contents of the container
	void clear();

	// get the nth element of the container (const)
	const CFileDescription& operator[](uint32 idx) const;

	// get the nth element of the container (non-const)
	CFileDescription& operator[](uint32 idx);

	// remove the 'n'th element from a file description container
	void removeFile(uint32 idx);

	// remove the provided string if found at the beginning of FileName
	void stripFilename(const std::string& header);

private:
	typedef std::vector<CFileDescription> TFileDescriptions;
	TFileDescriptions _FileDescriptions;
};


//-------------------------------------------------------------------------------------------------
#endif
