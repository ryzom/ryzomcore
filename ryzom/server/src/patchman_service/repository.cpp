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

// nel
#include "nel/misc/common.h"

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

// From spa_server_patch_applier.cpp
extern void writeVersionFile(const NLMISC::CSString& fileName, uint32 version);
extern uint32 readVersionFile(const NLMISC::CSString& fileName);

NLMISC::CSString getRepositoryIndexFileName(const NLMISC::CSString& repositoryName)
{
	return "repository_"+repositoryName+".idx";
}

uint32 getFileVersion(const NLMISC::CSString& fileName)
{
	// start at the back of the file name and scan forwards until we find a '/' or '\\' or ':' or a digit
	uint32 i= (uint32)fileName.size();
	while (i--)
	{
		char c= fileName[i];

		// if we've hit a directory name separator then we haven't found a version number so drop out
		if (c=='/' || c=='\\' || c==':')
			return ~0u;

		// if we've found a digit then construct the rest of the version number and return
		if (isdigit(c))
		{
			uint32 firstDigit= i;
			while (firstDigit!=0 && isdigit(fileName[firstDigit-1]))
			{
				--firstDigit;
			}
			return fileName.leftCrop(firstDigit).left(i-firstDigit+1).atoui();
		}
	}

	// default to our 'invalid' value
	return ~0u;
}

CHashKeyMD5 safeGetMD5(const NLMISC::CSString& fileName)
{
	while (true)
	{
		try
		{
			CHashKeyMD5 result= NLMISC::getMD5(fileName);
			return result;
		}
		catch(...)
		{
			nlwarning("Exception thrown in getMD5(\"%s\") ... will try again in a few seconds",fileName.c_str());
			nlSleep(3);
		}
	}
}


//-----------------------------------------------------------------------------
// methods CRepository
//-----------------------------------------------------------------------------

CRepository::CRepository()
{
	_Version= 0;
}

bool CRepository::init(const NLMISC::CSString& name,const NLMISC::CSString& directory)
{
	_Name= name.unquoteIfQuoted();
	_TargetDirectory= NLMISC::CPath::standardizePath(directory.unquoteIfQuoted());

	nldebug("Repository %s: %s",_Name.c_str(),_TargetDirectory.c_str());

	// check whether the target directory exists
	if (!NLMISC::CFile::isDirectory(_TargetDirectory))
	{
		// the directory didn't exist so try to create it...
		NLMISC::CFile::createDirectoryTree(_TargetDirectory);
		DROP_IF(!NLMISC::CFile::isDirectory(_TargetDirectory),"Failed to create target directory: \""+_TargetDirectory+"\"",return false);
	}

	// if we have a saved file that gives timestamp / size / checksum correspondances then load it
	NLMISC::CSString index;
	NLMISC::CSString indexFileName= _TargetDirectory+getRepositoryIndexFileName(_Name);
	if (NLMISC::CFile::fileExists(indexFileName))
	{
		index.readFromFile(indexFileName);
	}
	if (!index.empty())
	{
		nlinfo("GUSREP_Reading index file: %s",indexFileName.c_str());
		NLMISC::CVectorSString lines;
		index.splitLines(lines);
		for (uint32 i=0;i<lines.size();++i)
		{
			// get hold of the line and strip off comments and spurious blanks
			NLMISC::CSString line= lines[i].splitTo("//").strip();
			if (line.empty()) continue;

			// see if this is a special line of some sort
			if (line.left(1)=="*")
			{
				line=line.leftCrop(1).strip();
				NLMISC::CSString keyword= line.firstWord(true);
				if (keyword=="PatchVersion")
				{
					uint32 version= line.strip().atoui();
					DROP_IF(version==0,NLMISC::toString("Skipping line %d because expected to find version number but found: '%s'",i,line.c_str()),continue);
					_Version= version;
					nlinfo("Repository - Version number found in save file: %u",version);
				}
				continue;
			}

			// break the line down into constituent parts
			uint32 fileSize= line.strtok(" \t").atoi();
			uint32 fileTime= line.strtok(" \t").atoi();
			NLMISC::CHashKeyMD5 checksum;
			checksum.fromString(line.strtok(" \t"));
			NLMISC::CSString fileName= line.strip();

			// make sure that the text in the line was valid and that the file on disk looks like the one in our record
			DROP_IF(fileName.empty(),"Skipping line due to parse error: "+lines[i],continue);
			DROP_IF(_Files.find(fileName)!=_Files.end(),"Skipping line due to repeated file name: "+lines[i],continue);
			if (!NLMISC::CFile::fileExists(_TargetDirectory+fileName))	continue;
			if (NLMISC::CFile::getFileSize(_TargetDirectory+fileName)!=fileSize)	continue;
			if (NLMISC::CFile::getFileModificationDate(_TargetDirectory+fileName)!=fileTime)	continue;

			// add the result to our map of files
			_Files[fileName].set(fileSize,fileTime,checksum);
		}
		nlinfo("%d lines read from file, %d retained as pertinent",lines.size(),_Files.size());
	}

	// scan the target directory looking for updates
	update();

	// housekeeping and return with success
	return true;
}

void CRepository::updateFile(NLMISC::CSString fileName)
{
	nldebug(("GUSREP_Updating repository entry for file: '"+fileName+"'").c_str());

	// if the name of the file that has changed contains the target directory name then crop it
	if (fileName.left((uint32)_TargetDirectory.size())==_TargetDirectory)
	{
		fileName=fileName.leftCrop((uint32)_TargetDirectory.size());
	}

	// lookup the file in the map
	TFiles::iterator fileIt= _Files.find(fileName);
	BOMB_IF(fileIt== _Files.end(),"Failed to IDENTIFY the file that I have been asked to update: '"+fileName+"' in my map",return);

	// make sure the file exists on the disk
	BOMB_IF(!NLMISC::CFile::fileExists(_TargetDirectory+fileName),"Failed to LOCATE the file that I have been asked to update: '"+fileName+"' on the disk",return);

	for (bool done=false;!done;)
	{
		NLMISC::CSString fullFileName= _TargetDirectory+fileName;
		try
		{
			fileIt->second.FileSize= NLMISC::CFile::getFileSize(fullFileName);
			fileIt->second.FileTime= NLMISC::CFile::getFileModificationDate(fullFileName);
			fileIt->second.Checksum= safeGetMD5(fullFileName);
			done= true;
		}
		catch(...)
		{
			nlwarning("Failed to update file '%s' ... waiting a few seconds before trying again ...",fullFileName.c_str());
			nlSleep(2000);
		}
	}
}

void CRepository::addFileStub(NLMISC::CSString fileName)
{
	nldebug(("GUSREP_Adding repository stub for file: '"+fileName+"'").c_str());

	// if the name of the file that has changed contains the target directory name then crop it
	if (fileName.left((uint32)_TargetDirectory.size())==_TargetDirectory)
	{
		fileName=fileName.leftCrop((uint32)_TargetDirectory.size());
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

	// read the version number from the 'version' file and drop out immediately if the version is unchanged
	uint32 versionOnDisk= readVersionFile(_TargetDirectory+"version");
	if (versionOnDisk==_Version)
		return result;

	// scan the target directory to determine the files that it contains...
	nlinfo("GUSREP_Scanning for files in directory: %s",_TargetDirectory.c_str());
	CFileDescriptionContainer fdc;
	fdc.addFileSpec(_TargetDirectory+"*",true);

	// update the file index from the files found in the directory...
	nldebug("GUSREP_Checking index for updates",_TargetDirectory.c_str());
	for (uint32 i=0;i<fdc.size();++i)
	{
		// get a refference to the file description for this iteration
		CFileDescription& theFile= fdc[i];

		// get hold of the file name for the next file
//		CSString fileName= NLMISC::CFile::getFilename(theFile.FileName);
		CSString fileName= theFile.FileName.leftCrop((uint32)_TargetDirectory.size());

		// extract the version number from the file name and skip the file if it's too recent or the version number was invalid
		uint32 fileVersion= getFileVersion(fileName);
		if (fileVersion>versionOnDisk)
			continue;

		// check whether this is a file that should already be in my index
		if (fileVersion<=_Version)
		{
			// make sure we already had an entry for this file
			BOMB_IF(_Files.find(fileName)==_Files.end(),"Big nasty problem - The following file was not in my index but exists on the disk!: "+fileName,return result);

			// get hold of a refference to this file's entry in the index
			CFilesMapEntry& mapEntry= _Files[fileName];

			// make sure this file hasn't changed size...
			BOMB_IF(mapEntry.FileSize!=theFile.FileSize,"Big nasty problem - The following file was already in my index but size has changed!: "+fileName,return result);

			// if the file's timestamp has changed then we need to make sure the checksum hasn't
			if (mapEntry.FileTime!=theFile.FileTimeStamp)
			{
				BOMB_IF(safeGetMD5(theFile.FileName)!=mapEntry.Checksum,"Big nasty problem - The following file was already in my index but checksum has changed!: "+fileName,return result);
			}

			// the file matches OK so just ignore it
			continue;
		}

		// get hold of a refference to this file's entry in the index (create a new entry if need be)
		CFilesMapEntry& mapEntry= _Files[fileName];

		// check whether the info in the file description corresponds to the index entry...
		if (mapEntry.FileSize!=theFile.FileSize || mapEntry.FileTime!=theFile.FileTimeStamp)
		{
			// the file index entry is not up to date so update it
			nldebug("GUSREP_Updating file index entry for file: %s",fileName.c_str());
			mapEntry.FileSize= theFile.FileSize;
			mapEntry.FileTime= theFile.FileTimeStamp;
			mapEntry.Checksum= safeGetMD5(theFile.FileName);

			// write the file index back to disk with this new record
			writeIndexFile();
			++result;
		}
	}

	_Version= versionOnDisk;
	writeIndexFile();
	return result;
}

void CRepository::writeIndexFile()
{
//	nldebug("GUSREP_Writing index: %s",(_TargetDirectory+getRepositoryIndexFileName(_Name)).c_str());
	NLMISC::CSString fileIndex;
	iterator it= _Files.begin();
	iterator itEnd= _Files.end();
	fileIndex+= NLMISC::toString("*PatchVersion %d\n",_Version);
	for(;it!=itEnd;++it)
	{
		fileIndex+= NLMISC::toString("%10d %10d %16s %s\n",it->second.FileSize,it->second.FileTime,it->second.Checksum.toString().c_str(),it->first.c_str());
	}
	fileIndex.writeToFile(_TargetDirectory+getRepositoryIndexFileName(_Name));
}

// display info about the repository (version number, directory names, etc)
void CRepository::display(NLMISC::CLog& log) const
{
	log.displayNL("Directory: %s (%u files found)",_TargetDirectory.c_str(),_Files.size());
	log.displayNL("Index file name: %s",(_TargetDirectory+getRepositoryIndexFileName(_Name)).c_str());
	log.displayNL("Version: %u",_Version);
}

uint32 CRepository::getVersion() const
{
	return _Version;
}

void CRepository::setVersion(uint32 version)
{
	writeVersionFile(_TargetDirectory+"version", version);
	_Version= version;
}

uint32 CRepository::size() const
{
	return (uint32)_Files.size();
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

void CRepository::fillShortList(PATCHMAN::TFileInfoVector	&files) const
{
	// start by clearing out any previous contents in the files vector
	files.clear();

	// iterate over the repository adding files to the files vector
	const_iterator it= _Files.begin();
	const_iterator itEnd= _Files.end();
	for (;it!=itEnd;++it)
	{
		// skip files that we haven't finished building info on
		if (it->second.FileSize==0)
			continue;

		// append a new entry to the vector
		vectAppend(files);
		// setup data for the (new) back vector entry
		files.back().FileName=it->first;
		files.back().Checksum=it->second.Checksum;
		nldebug("GUSREP_sending info on file: '%s'",files.back().FileName.c_str());
	}
}
