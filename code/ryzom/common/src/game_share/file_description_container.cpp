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

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdpch.h"

#include "nel/misc/path.h"
#include "nel/misc/algo.h"

#include "utils.h"
#include "file_description_container.h"

#ifdef NL_OS_WINDOWS
#include <time.h>
#include <sys/types.h>
//#include <sys/stat.h>
#include <stdio.h>
#endif


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// methods CFileDescription
//-------------------------------------------------------------------------------------------------

CFileDescription::CFileDescription(const string& name,uint32 time,uint32 size)
{
	FileName=name;
	FileTimeStamp=time;
	FileSize=size;
}

bool CFileDescription::set(const string& name)
{
	FileName=name;

	// if the file doesn't exist then zero out time stamp and size and return false
	if (!NLMISC::CFile::fileExists(name))
	{
		FileTimeStamp=0;
		FileSize=0;
		return false;
	}

	// setup timestamp and size for the file
	FileTimeStamp=NLMISC::CFile::getFileModificationDate(name);
	FileSize=NLMISC::CFile::getFileSize(name);

	return true;
}

void CFileDescription::serial(NLMISC::IStream& stream)
{
	stream.serial(FileName);
	stream.serial(FileTimeStamp);
	stream.serial(FileSize);
}

CSString CFileDescription::toString(uint32 maxFileNameLen) const
{
	return NLMISC::toString("%*s (size: %u, time stamp: %s)",maxFileNameLen,FileName.c_str(),FileSize,IDisplayer::dateToHumanString(FileTimeStamp));
}

bool CFileDescription::operator<(const CFileDescription& other) const
{
	return FileName<other.FileName;
}


//-------------------------------------------------------------------------------------------------
// methods CFileDescriptionContainer
//-------------------------------------------------------------------------------------------------

// add a specific file to the container
void CFileDescriptionContainer::addFile(const CFileDescription& fileDescription)
{
	_FileDescriptions.push_back(fileDescription);
}

void CFileDescriptionContainer::addFile(const string& fileName, uint32 timeStamp, uint32 size)
{
	CFileDescription& theFileDescription= vectAppend(_FileDescriptions);
	theFileDescription.FileName=		fileName;
	theFileDescription.FileTimeStamp=	timeStamp;
	theFileDescription.FileSize=		size;
}

void CFileDescriptionContainer::addFile(const string& fileName)
{
//#ifdef NL_OS_WINDOWS
//
//	struct _stat buffer;
//	uint32 result= _stat(fileName.c_str(),&buffer);
//	if (result==0)
//	{
//		addFile(fileName, uint32(buffer.st_mtime), buffer.st_size);
//	}
//
//#else

	if (CFile::fileExists(fileName))
	{
		addFile(fileName,CFile::getFileModificationDate(fileName),CFile::getFileSize(fileName));
	}

//#endif
}

void CFileDescriptionContainer::addFileSpec(const string& fileSpec,bool recurse)
{
	// extract path and wildcard from fileSpec
	string theWildcard=NLMISC::CFile::getFilename(fileSpec);
	string path=NLMISC::CFile::getPath(fileSpec);
	std::vector<std::string> wildcards;
	wildcards.push_back(theWildcard.empty()? "*": theWildcard);
	addFiles(path,wildcards,recurse);
}

void CFileDescriptionContainer::addFiles(const std::string& directory, const std::vector<std::string>& wildcards, bool recurse)
{
	H_AUTO(fdcAddFiles)
	std::string path= directory.empty()? ".": directory;

	// build a list of all files in the matching directory
	vector<string> rawFileNames;
	{
		H_AUTO(fdcAddFilesGetPathContents)
		NLMISC::CPath::getPathContent(path.c_str(),recurse,false,true,rawFileNames);
	}

	// extract the files that match the given wildcard and build a result vector
	{
		H_AUTO(fdcAddFilesTestWildcards)
		for (uint32 i=(uint32)rawFileNames.size();i--;)
		{
			uint32 j;
			for (j=0;j<wildcards.size();++j)
			{
				if (testWildCard(NLMISC::CFile::getFilename(rawFileNames[i]),wildcards[j]))
					break;
			}
			if (j!=wildcards.size())
			{
				H_AUTO(fdcAddFilesGetFileProperties)
				addFile(rawFileNames[i]);
			}
		}
	}
}

void CFileDescriptionContainer::addFiles(const std::string& directory, const NLMISC::CVectorSString& wildcards, bool recurse)
{
	// note that a CVectorSString is guaranteed to be bitwise identical to a vector of std::string
	addFiles(directory,reinterpret_cast<const std::vector<std::string>&>(wildcards),recurse);
}

void CFileDescriptionContainer::addFiles(const CFileDescriptionContainer& other)
{
	// note: the following can doubtless be done with a single tsl operation (eg +=)
	// but I don't have an stl refference to hand so doing it longhand...

	// make room in the file description container for the data from the other object
	uint32 otherSize= (uint32)other._FileDescriptions.size();
	uint32 oldSize= (uint32)_FileDescriptions.size();
	uint32 newSize= oldSize + otherSize;
	_FileDescriptions.resize(newSize);

	// prepare to copy from one vector to the other
	TFileDescriptions::iterator destIt	= _FileDescriptions.begin()+oldSize;
	TFileDescriptions::const_iterator srcIt	= other._FileDescriptions.begin();
	TFileDescriptions::const_iterator itEnd	= other._FileDescriptions.end();

	// do the copy
	while (srcIt!=itEnd)
	{
		*destIt= *srcIt;
		++srcIt;
		++destIt;
	}
}

void CFileDescriptionContainer::display(NLMISC::CLog* log) const
{
	nlassert(log!=NULL);

	// determine the length of the longest file name
	uint32 longest=10;
	for (uint32 i=0;i<_FileDescriptions.size();++i)
		longest=max(longest,(uint32)_FileDescriptions[i].FileName.size());

	// build a vector of strings to display
	vector<string> lines;
	for (uint32 i=0;i<_FileDescriptions.size();++i)
		lines.push_back(NLMISC::toString("%-*s %12d %s",longest,
			_FileDescriptions[i].FileName.c_str(),
			_FileDescriptions[i].FileSize,
			IDisplayer::dateToHumanString(_FileDescriptions[i].FileTimeStamp)));

	// sort the lines to make a nice alphabetical list
	sort(lines.begin(),lines.end());

	// display a nice banner
	log->displayNL("%-*s %12s %12s",longest,"File_Name","File_Size","File_Time");

	// display the file list
	for (uint32 i=0;i<_FileDescriptions.size();++i)
		log->displayNL(lines[i].c_str());
}

void CFileDescriptionContainer::serial(NLMISC::IStream& stream)
{
	stream.serialCont(_FileDescriptions);
}

uint32 CFileDescriptionContainer::size() const
{
	return (uint32)_FileDescriptions.size();
}

bool CFileDescriptionContainer::empty() const
{
	return _FileDescriptions.empty();
}

void CFileDescriptionContainer::clear()
{
	_FileDescriptions.clear();
}

const CFileDescription& CFileDescriptionContainer::operator[](uint32 idx) const
{
	return (_FileDescriptions[idx]);
}

CFileDescription& CFileDescriptionContainer::operator[](uint32 idx)
{
	return (_FileDescriptions[idx]);
}

// remove the 'n'th element from a file description container
void CFileDescriptionContainer::removeFile(uint32 idx)
{
	BOMB_IF(idx>=_FileDescriptions.size(),"trying to remove files beyond the end of the file description vector",return);
	_FileDescriptions[idx]=_FileDescriptions.back();
	_FileDescriptions.pop_back();
}


// remove the provided string if found at the beginning of FileName
void CFileDescriptionContainer::stripFilename(const std::string& header)
{
	for (std::vector<CFileDescription>::iterator it=_FileDescriptions.begin(); it!=_FileDescriptions.end(); ++it)
	{
		it->stripFilename(header);
	}
}


//-------------------------------------------------------------------------------------------------
