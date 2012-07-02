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

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"

// local
#include "repository.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-------------------------------------------------------------------------------------------------
// constants & utilities
//-------------------------------------------------------------------------------------------------

NLMISC::CSString getRepositoryIndexFileName(const NLMISC::CSString& repositoryName)
{
	return "repository_"+repositoryName+".idx";
}


//-----------------------------------------------------------------------------
// methods CRepository
//-----------------------------------------------------------------------------

bool CRepository::init(const NLMISC::CSString& name,const NLMISC::CSString& directory,const NLMISC::CSString& filespec,const NLMISC::CSString& lockFilespecs)
{
	_Name= name.unquoteIfQuoted();
	_TargetDirectory= NLMISC::CPath::standardizePath(directory.unquoteIfQuoted());
	filespec.splitBySeparator('|',_Filespec);
	if (_Filespec.empty()) _Filespec.push_back("*");
	lockFilespecs.splitBySeparator('|',_LockFilespecs);

	nldebug("Repository %s: %s",_Name.c_str(),_TargetDirectory.c_str());
	nldebug("- filespec: %s",NLMISC::CSString().join(_Filespec,';').c_str());
	nldebug("- avoid filespec: %s",NLMISC::CSString().join(_LockFilespecs,';').c_str());

	// check whether the target directory exists
	if (!NLMISC::CFile::isDirectory(_TargetDirectory))
	{
		// the directory didn't exist so try to create it...
		NLMISC::CFile::createDirectoryTree(_TargetDirectory);
		DROP_IF(!NLMISC::CFile::isDirectory(_TargetDirectory),"Failed to create target directory: \""+_TargetDirectory+"\"",return false);
	}

	// if we have a saved file that gives timestamp / size / checksum correspondances then load it
	NLMISC::CSString index;
	if (NLMISC::CFile::fileExists(_TargetDirectory+getRepositoryIndexFileName(_Name)))
	{
		index.readFromFile(_TargetDirectory+getRepositoryIndexFileName(_Name));
	}
	if (!index.empty())
	{
		nlinfo("GUSREP_Reading index file: %s",(_TargetDirectory+getRepositoryIndexFileName(_Name)).c_str());
		NLMISC::CVectorSString lines;
		index.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
		{
			// get hold of the line and strip off comments and spurious blanks
			NLMISC::CSString line= lines[i].splitTo("//").strip();
			if (line.empty()) continue;

			// break the line down into constituent parts
			uint32 fileSize= line.strtok(" \t").atoi();
			uint32 fileTime= line.strtok(" \t").atoi();
			NLMISC::CHashKeyMD5 checksum;
			checksum.fromString(line.strtok(" \t"));
			NLMISC::CSString fileName= line.strip();

			// make sure that the text in the line was valid and that the file name existed ok
			DROP_IF(fileName.empty(),"Skipping line due to parse error: "+lines[i],continue);
			DROP_IF(_Files.find(fileName)!=_Files.end(),"Skipping line due to repeated file name: "+lines[i],continue);
			if (!NLMISC::CFile::fileExists(_TargetDirectory+fileName))	continue;

			// add the result to our map of files
			_Files[fileName].set(fileSize,fileTime,checksum);
		}
	}

	// scan the target directory looking for updates
	update();

	// housekeeping and return with success
	return true;
}

void CRepository::updateFile(NLMISC::CSString fileName)
{
	nldebug(("Updating repository entry for file: '"+fileName+"'").c_str());

	// if the name of the file that has changed contains the target directory name then crop it
	if (fileName.left(_TargetDirectory.size())==_TargetDirectory)
	{
		fileName=fileName.leftCrop(_TargetDirectory.size());
	}

	// lookup the file in the map
	TFiles::iterator fileIt= _Files.find(fileName);
	BOMB_IF(fileIt== _Files.end(),"Failed to IDENTIFY the file that I have been asked to update: '"+fileName+"'",return);

	// make sure the file exists on the disk
	BOMB_IF(!NLMISC::CFile::fileExists(_TargetDirectory+fileName),"Failed to LOCATE the file that I have been asked to update: '"+fileName+"'",return);

	fileIt->second.FileSize= NLMISC::CFile::getFileSize(_TargetDirectory+fileName);
	fileIt->second.FileTime= NLMISC::CFile::getFileModificationDate(_TargetDirectory+fileName);
	fileIt->second.Checksum= NLMISC::getMD5(_TargetDirectory+fileName);
}

void CRepository::addFileStub(NLMISC::CSString fileName)
{
	nldebug(("Adding repository stub for file: '"+fileName+"'").c_str());

	// if the name of the file that has changed contains the target directory name then crop it
	if (fileName.left(_TargetDirectory.size())==_TargetDirectory)
	{
		fileName=fileName.leftCrop(_TargetDirectory.size());
	}

	// make sure the file didn't already exist in the map
	TFiles::iterator fileIt= _Files.end();
	fileIt=_Files.find(fileName);
	BOMB_IF(fileIt!= _Files.end(),"Failed to add stub for file that already exists: '"+fileName+"'",return);

	// create the new map entry and set properties to 0
	_Files[fileName].FileSize= 0;
	_Files[fileName].FileTime= 0;
}

uint32 CRepository::update()
{
	// setup a variable to hold our return value
	uint32 result= 0;

	// make sure there are no 'avoid' files about
	CFileDescriptionContainer fdc;
	for (uint32 i=0;i<_LockFilespecs.size();++i)
	{
		fdc.addFileSpec(_LockFilespecs[i]);
	}
	if (!fdc.empty())
	{
		// some avoid files have been found so build a list of them
		NLMISC::CSString s;
		for (uint32 i=0;i<fdc.size();++i)
		{
			if (!s.empty())	s+= " | ";
			s+= fdc[i].FileName;
		}
		if (s!=_BlockingFiles)
		{
			// the avoid list has changed so re-display it and record it for safe keeping
			nldebug("%s: Waiting for the following files to be removed: %s",_Name.c_str(),s.c_str());
			_BlockingFiles= s;
		}
		return 0;
	}

	// clear out the blocking files string because there clearly are none if we made it this far.
	_BlockingFiles.clear();

	// scan the target directory to determine the files that it contains...
//	nlinfo("GUSREP_Scanning for files in directory: %s",_TargetDirectory.c_str());
	fdc.clear();
	fdc.addFiles(_TargetDirectory,_Filespec,true);

	// update the file index from the files found in the directory...
//	nlinfo("GUSREP_Checking index for updates",_TargetDirectory.c_str());
	for (uint32 i=0;i<fdc.size();++i)
	{
		// get a refference to the file description for this iteration
		CFileDescription& theFile= fdc[i];

		// get hold of the file name for the next file
//		CSString fileName= NLMISC::CFile::getFilename(theFile.FileName);
		CSString fileName= theFile.FileName.leftCrop(_TargetDirectory.size());

		// if this is the index file then skip it
		if (fileName==getRepositoryIndexFileName(_Name))
			continue;

		// get hold of a refference to this file's entry in the index (create a new entry if need be)
		CFilesMapEntry& mapEntry= _Files[fileName];

		// check whether the info in the file description corresponds to the index entry...
		if (mapEntry.FileSize!=theFile.FileSize || mapEntry.FileTime!=theFile.FileTimeStamp)
		{
			// the file index entry is not up to date so update it
			nlinfo("GUSREP_Updating file index entry for file: %s",fileName.c_str());
			mapEntry.FileSize= theFile.FileSize;
			mapEntry.FileTime= theFile.FileTimeStamp;
			mapEntry.Checksum= NLMISC::getMD5(theFile.FileName);

			// write the file index back to disk with this new record
			writeIndexFile();
			++result;
		}
	}

	return result;
}

void CRepository::writeIndexFile()
{
	nlinfo("GUSREP_Writing index: %s",(_TargetDirectory+getRepositoryIndexFileName(_Name)).c_str());
	NLMISC::CSString fileIndex;
	iterator it= _Files.begin();
	iterator itEnd= _Files.end();
	for(;it!=itEnd;++it)
	{
		fileIndex+= NLMISC::toString("%10d %10d %16s %s\n",it->second.FileSize,it->second.FileTime,it->second.Checksum.toString().c_str(),it->first.c_str());
	}
	fileIndex.writeToFile(_TargetDirectory+getRepositoryIndexFileName(_Name));
}

uint32 CRepository::size() const
{
	return _Files.size();
}

const CRepository::CFilesMapEntry& CRepository::operator[](const NLMISC::CSString& key) const
{
	return const_cast<CRepository*>(this)->_Files[key];
}

CRepository::iterator CRepository::find(const NLMISC::CSString& key)
{
	return _Files.find(key);
}

CRepository::const_iterator CRepository::find(const NLMISC::CSString& key) const
{
	return _Files.find(key);
}

CRepository::iterator CRepository::begin()
{
	return _Files.begin();
}

CRepository::const_iterator CRepository::begin() const
{
	return _Files.begin();
}

CRepository::iterator CRepository::end()
{
	return _Files.end();
}

CRepository::const_iterator CRepository::end() const
{
	return _Files.end();
}

void CRepository::fillShortList(std::vector<GUS_SCM::TFileRecord>	&files) const
{
	// start by clearing out any previous contents in the files vector
	files.clear();

	// iterate over the repository adding files to the files vector
	const_iterator it= _Files.begin();
	const_iterator itEnd= _Files.end();
	for (;it!=itEnd;++it)
	{
		// append a new entry to the vector
		vectAppend(files);
		// setup data for the (new) back vector entry
		files.back().setFileName(it->first);
		files.back().setChecksum(it->second.Checksum);
		nlinfo("sending info on file: '%s'",files.back().getFileName().c_str());
	}
}
