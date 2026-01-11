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



#include <stdio.h>
//#include <direct.h>
#include <io.h>

#include "nel/misc/types_nl.h"
#include "nel/misc/path.h"
#include "nel/misc/file.h"
#include "nel/misc/i_xml.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/config_file.h"
#include "nel/net/service.h"


#include "game_share/xml.h"
#include "ai_files.h"
#include "ai_manager.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;


//--------------------------------------------------------------------------------
// LOCAL GLOBALS - EQUIVALENT TO SINGLETON DATA
//--------------------------------------------------------------------------------

class CFileInfo
{
public:
	std::string Source;
	uint Size;
	uint DateTime;

	void clear()
	{
		Source.clear();
		Size=0;
		DateTime=0;
	}
};

static CFileInfo mgrObjInfo[RYAI_AI_MANAGER_MAX_MANAGERS];


//--------------------------------------------------------------------------------
// scan() METHOD
//--------------------------------------------------------------------------------

// clear out the data strustures and rescan the directories listed in the config
// file for source and object files

void CAIFiles::scan()
{
	uint i;

	// clear out out the internal data tables before we begin
	for (i=0;i<CAIManager::maxManagers();i++)
		mgrObjInfo[i].clear();
	CAIManager::liberateUnassignedManagers();

	// scan the obj and sav paths for files
	_scanObjAndSavFiles();

	// iterate through the paths specified in the config file looking for src Files
	std::vector<std::string> paths=srcPaths();
	for (i = 0; i < paths.size(); i++)
	{
		nlinfo("SCANNING: %s",paths[i].c_str());
		_scanSrcFiles(paths[i]);
	}

	// run through the managers in the AI_Manager singleton, updating parameters
	for (i=0;i<CAIManager::numManagers();i++)
	{
		CAIManager *mgr=CAIManager::getManagerByIdx(i);
		uint mgrId=mgr->id();
		nlinfo("Manager: %04d: %s",mgrId,mgr->name().c_str());
	}
}

//--------------------------------------------------------------------------------
// clean() METHOD
//--------------------------------------------------------------------------------

void CAIFiles::clean(sint mgrId)
{
	remove(fullObjFileName(mgrId).c_str());
}

//--------------------------------------------------------------------------------
// FILE NAME HANDLING METHODS
//--------------------------------------------------------------------------------

//--------------------------------------------------------------------------------
// source file name, extension, etc

// the paths for source files
std::vector<std::string> CAIFiles::srcPaths()
{
	std::vector<std::string> v;
	try
	{
		CConfigFile::CVar& var = IService::getInstance()->ConfigFile.getVar("SrcPaths");
		for (uint i = 0; i < var.size(); i++)
			v.push_back(CPath::standardizePath(var.asString(i)));
	}
	catch(EUnknownVar &) 
	{
		nlwarning("<SrcPaths> missing variables in config file, using '.'");
		v.push_back(std::string("./"));
	}

	return v;
}

// the standard src extension
std::string CAIFiles::srcExtension()
{
	return std::string("primitive");
}

// without path or extension
std::string CAIFiles::srcName(sint mgrId)
{
	return CFile::getFilenameWithoutExtension(fullSrcFileName(mgrId));
}

// without path
std::string CAIFiles::srcFileName(sint mgrId)
{
	return CFile::getFilename(fullSrcFileName(mgrId));
}

// without extension
std::string CAIFiles::fullSrcName(sint mgrId)
{
	return 
		CFile::getPath(fullSrcFileName(mgrId))+
		CFile::getFilenameWithoutExtension(fullSrcFileName(mgrId));
}

// with path and extension
std::string CAIFiles::fullSrcFileName(sint mgrId)
{
	if (uint(mgrId)>=CAIManager::maxManagers())
	{
		nlwarning("CAIFiles::fullSrcFileName(mgrId): mgrId %d not in range 0..%d",mgrId,CAIManager::maxManagers()-1);
		return string();
	}

	return mgrObjInfo[mgrId].Source;
}


//--------------------------------------------------------------------------------
// object file name, extension, etc

// the path for obj files
std::string CAIFiles::objPath()
{
	std::string s;

	// get the object path from the config file
	try
	{
		s=IService::getInstance()->ConfigFile.getVar("ObjPath").asString();
	}
	catch(EUnknownVar &) 
	{
		nlwarning("<ObjPath> missing variables in config file, using '.'");
	}

	// if we haven't found a path use the current directory
	if (s.empty())
		s="./";

	return CPath::standardizePath(s);
}

// the standard obj file extension
std::string CAIFiles::objExtension()
{
	return std::string("aimgr_obj");
}

// without path or extension
std::string CAIFiles::objName(sint mgrId)
{
	if (uint(mgrId)>=CAIManager::maxManagers())
	{
		nlwarning("CAIFiles::fullSrcFileName(mgrId): mgrId %d not in range 0..%d",mgrId,CAIManager::maxManagers()-1);
		return string();
	}

	std::string s;
	s+=(mgrId/1000)+'0';
	s+=(mgrId%1000)/100+'0';
	s+=(mgrId%100)/10+'0';
	s+=(mgrId%10)+'0';

	return s;
}

// without extension
std::string CAIFiles::objFileName(sint mgrId)
{
	// if the objName() method fails to generate a file name return an empty string
	std::string s=objName(mgrId);
	if (s.empty())
		return s;

	return s+'.'+objExtension();
}

// with path and extension
std::string CAIFiles::fullObjFileName(sint mgrId)
{
	// if the objFileName() method fails to generate a file name return an empty string
	std::string s=objFileName(mgrId);
	if (s.empty())
		return s;

	return objPath()+s;
}



//--------------------------------------------------------------------------------
// saved data file name, extension, etc

// the path for sav files
std::string CAIFiles::savPath()
{
	std::string s;

	// get the sav path from the config file
	try
	{
		s=IService::getInstance()->ConfigFile.getVar("SavPath").asString();
	}
	catch(EUnknownVar &) 
	{
		nlwarning("<SavPath> missing variables in config file, using <ObjPath>");
		return objPath();
	}

	// if we haven't found a path use the current directory
	if (s.empty())
		s="./";

	return CPath::standardizePath(s);
}

// the standard sav file extension
std::string CAIFiles::savExtension()
{
	return std::string("aimgr_sav");
}

// without path or extension
std::string CAIFiles::savName(sint mgrId)
{
	return objName(mgrId);
}

// without extension
std::string CAIFiles::savFileName(sint mgrId)
{
	// if the savName() method fails to generate a file name return an empty string
	std::string s=savName(mgrId);
	if (s.empty())
		return s;

	return s+'.'+savExtension();
}

// with path and extension
std::string CAIFiles::fullSavFileName(sint mgrId)
{
	// if the savName() method fails to generate a file name return an empty string
	std::string s=savName(mgrId);
	if (s.empty())
		return s;

	return savPath()+s;
}

//--------------------------------------------------------------------------------
// CAIFiles PUBLIC METHODS - FOR READING & WRITING OBJ FILES
//--------------------------------------------------------------------------------

void CAIFiles::writeObjFile(sint mgrId)
{
	// write the output file
	NLMISC::COFile file;
	bool fileIsOpen=false;
	try
	{
		NLMISC::COXml output;

		if (!file.open(fullObjFileName(mgrId))) throw(NLMISC::Exception());
		fileIsOpen=true;
		if (output.init (&file, "1.0"))
		{
			output.xmlPush("AI_MANAGER");
			std::string src=CAIFiles::srcName(mgrId);
			((NLMISC::IStream*)&output)->serial(src);
			CAIManager::getManagerById(mgrId)->MgrDfnRootNode.serial(output);
			output.xmlPop();
			output.flush ();
		}
	}
 	catch (NLMISC::Exception &)
	{
		nlwarning("CAIFiles::writeObjFile(): Failed to write the output file: %s",fullObjFileName(mgrId).c_str());
	}
	// close file outside exception handling in case exception thrown somewhere strange
	if (fileIsOpen)
		file.close();
}

//--------------------------------------------------------------------------------
// CAIFiles PRIVATE METHODS - FOR ADDING FILE RECORDS TO RELAVENT DATA STRUCTURES
//--------------------------------------------------------------------------------

void CAIFiles::_addObjFile(sint mgrId, std::string fullFileName, uint timestamp)
{
	// read the input file
	NLMISC::CIFile file;
	bool fileIsOpen=false;
	try
	{
		NLMISC::CIXml input;

		if (!file.open(fullObjFileName(mgrId))) throw(NLMISC::Exception());
		fileIsOpen=true;
		if (input.init (file))
		{
			input.xmlPush("AI_MANAGER");
			((NLMISC::IStream*)&input)->serial(mgrObjInfo[mgrId].Source);
			CAIManager::getManagerById(mgrId)->MgrDfnRootNode.serial(input);
			input.xmlPop();
		}
	}
 	catch (NLMISC::Exception &)
	{
		nlwarning("CAIFiles::_addObjFile(): Failed to read the input file: %s",fullObjFileName(mgrId).c_str());
	}
	// close file outside exception handling in case exception thrown somewhere strange
	if (fileIsOpen)
		file.close();

	// ask the ai manager to initialise itself with name 'name' (fails if manager is running)
	if (!CAIManager::getManagerById(mgrId)->set(srcName(mgrId)))
	{
		nlwarning("Conflict between running manager %04d named '%s' and name in obj file '%s'",
			mgrId,CAIManager::getManagerById(mgrId)->name().c_str(),srcName(mgrId).c_str());
		return;
	}

	// assign the new record to the designated slot
	mgrObjInfo[mgrId].DateTime=timestamp;	// timestamp of obj file

	// update the manager flags
	CAIManager::getManagerById(mgrId)->setObjFileExists(true);
}


void CAIFiles::_addSavFile(sint mgrId, std::string fullFileName)
{
	// open and parse the file
	CxmlNode theXmlFile;
	if (!theXmlFile.read(fullFileName))
	{
		nlwarning("Failed to parse xml file: %s",fullFileName.c_str());
		return;
	}

	// make sure the main xml clause is of the correct type for an obj file
	if (theXmlFile.type()!=string("ai_manager_dynamic_data"))
	{
		nlwarning("Ignoring file %s because main clause is not 'ai_manager_dynamic_data'",fullFileName.c_str());
		return;
	}

	// extract the 'name' argument from the main clause
	std::string name=theXmlFile.arg("name");
	if (name.empty())
	{
		nlwarning("No manager name found in save file: %s",fullFileName.c_str());
		return;
	}

	// ask the ai manager to initialis itself with name 'name' (fails if manager is running)
	if (!CAIManager::getManagerById(mgrId)->set(name))
	{
		nlwarning("Conflict between running manager %04d named '%s' and name in sav file '%s'",
			mgrId,CAIManager::getManagerById(mgrId)->name().c_str(),name.c_str());
		return;
	}
}


void CAIFiles::_addSrcFile(std::string fileName,std::string fullFileName, uint timestamp)
{
	// generate a manger name from the file name
	std::string name=CFile::getFilenameWithoutExtension(fileName);

	// ask the CAIManager singleton for a manager id for this name
	sint mgrId = CAIManager::nameToId(name,true);
	if (mgrId==-1)
	{
		nlwarning("Failed to allocate a manager id for name '%s' (manager ids 0..%d)",
			name.c_str(), CAIManager::maxManagers()-1);
		return;
	}

	// make sure that we don't have another file already allocated to this manager
	if (!mgrObjInfo[mgrId].Source.empty() && fullSrcFileName(mgrId)!=fullFileName  && srcName(mgrId)!=NLMISC::CFile::getFilenameWithoutExtension(fullFileName))
	{
		nlwarning("File name conflict for: %s: %s != %s",
			name.c_str(),mgrObjInfo[mgrId].Source.c_str(),NLMISC::CFile::getFilenameWithoutExtension(fullFileName).c_str());
		return;
	}

	// assign the file name 
	mgrObjInfo[mgrId].Source=fullFileName;

	// update the manager flags
	if (timestamp>mgrObjInfo[mgrId].DateTime)
		CAIManager::getManagerById(mgrId)->setNeedCompile(true);
}

//--------------------------------------------------------------------------------
// CAIFiles PRIVATE METHODS - FOR DIRECTORIES FOR FILES
//--------------------------------------------------------------------------------

void CAIFiles::_scanSrcFiles(const std::string &path)
{
	//nlinfo("Scanning source directory: %s",path.c_str());

	// setup the directory scan
	_finddata_t fd;
	long searchHandle;
	if( (searchHandle = _findfirst( (path+"*").c_str(), &fd )) == -1L )
	{
		nlwarning("Nothing found in directory: %s",path.c_str());
		return;
	}

	// iterate through found entries in the directory
	do
	{
		// if we have a directory then recurse
		if ( (fd.attrib & _A_SUBDIR) != 0 )
		{
			// ignore '.' and '..' entries
			if (fd.name[0]!='.' || (fd.name[1]!=0 && (fd.name[1]!='.' || fd.name[2]!=0) ) )
				_scanSrcFiles(path+std::string(fd.name)+"/");
		}
		else
		{
			// we've found a file so have a look to see whether its one of ours
			std::string fileName=std::string(fd.name);
			if (CFile::getExtension(fileName)==srcExtension())
				_addSrcFile(fileName, path+fileName, uint32(fd.time_write));
		}
	}
	while( _findnext( searchHandle, &fd ) == 0 );

	// housekeeping
	_findclose( searchHandle );
}

void CAIFiles::_scanObjAndSavFiles()
{
	// setup to scan for obj files
	std::string path=objPath();
	nlinfo("Scanning directory: %s for *.%s",path.c_str(),objExtension().c_str());

	// iterate through manager ids looking for correspoding files
	for (uint i=0;i<CAIManager::maxManagers();i++)
	{
		// compose the file name
		std::string filename=path+objFileName(sint(i));
		_finddata_t fd;

		// look for the file and retrieve timestamp info if found
		long searchHandle;
		if( (searchHandle=_findfirst(filename.c_str(),&fd)) != -1L )
			_addObjFile(i, filename.c_str(), uint32(fd.time_write));
		_findclose( searchHandle );
	}

	// setup to scan for sav files
	path=savPath();
	nlinfo("Scanning directory: %s for *.%s",path.c_str(),savExtension().c_str());

	// iterate through manager ids looking for correspoding files
	for (uint j=0;j<CAIManager::maxManagers();j++)
	{
		// if we've already found the name then we don't bother looking for the sav file
		if (!CAIManager::getManagerById(j)->name().empty()) continue;

		// compose the file name
		std::string filename=path+savFileName(sint(j));
		_finddata_t fd;

		// look for the file
		long searchHandle;
		if( (searchHandle=_findfirst(filename.c_str(),&fd)) != -1L )
			_addSavFile(j,filename.c_str());
		_findclose( searchHandle );
	}
}

