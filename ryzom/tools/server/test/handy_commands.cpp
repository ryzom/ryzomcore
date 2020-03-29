/*
	Handy utility commands

	project: RYZOM / TEST

*/

//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------
#include "stdpch.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/algo.h"
#include "game_share/persistent_data.h"


//-----------------------------------------------------------------------------
// Handy utility commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(utils,pdrBin2xml,"convert a binary pdr file to xml","<input file name> <output file name>")
{
	if (args.size()!=2)
		return false;

	CPersistentDataRecord pdr;
	pdr.readFromBinFile(args[0].c_str());
	pdr.writeToTxtFile(args[1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdrXml2bin,"convert a text pdr file to binary","<input file name> <output file name>")
{
	if (args.size()!=2)
		return false;

	CPersistentDataRecord pdr;
	pdr.readFromTxtFile(args[0].c_str());
	pdr.writeToBinFile(args[1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,quickFileCompare,"compare 2 files (by comparing timestamp and size)","<file0> <file1>")
{
	if (args.size()!=2)
		return false;

	log.displayNL("comparing files ...");
	bool result= NLMISC::CFile::quickFileCompare(args[0], args[1]);
	log.displayNL("- %s",result?"Same":"Different");

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,thoroughFileCompare,"compare 2 files (by comparing data)","<file0> <file1> [<max mem footprint>]")
{
	if (args.size()!=2 && args.size()!=3)
		return false;

	bool result;
	log.displayNL("comparing files ...");

	if (args.size()==3)
	{
		uint32 size=atoi(args[2].c_str());
		if (size<2)
		{
			log.displayNL("The third parameter must be a value >= 2 : The following value is not valid: %s",args[2].c_str());
			return true;
		}
		result= NLMISC::CFile::thoroughFileCompare(args[0], args[1], size);
	}
	else
	{
		result= NLMISC::CFile::thoroughFileCompare(args[0], args[1]);
	}

	log.displayNL("- %s",result?"Same":"Different");
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,cd,"change directory or display current working directory","[<path>]")
{
	if (args.size()!=0 && args.size()!=1)
		return false;

	if (args.size()==1)
	{
		NLMISC::CPath::setCurrentPath(args[0].c_str());
	}

	log.displayNL("Current directory: %s",NLMISC::CPath::getCurrentPath().c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,md,"create a new directory (or directory tree)","<path>")
{
	if (args.size()!=1)
		return false;

	NLMISC::CFile::createDirectoryTree(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,copyFile,"copy a file","<src> <dest>")
{
	if (args.size()!=2)
		return false;

	NLMISC::CFile::copyFile(args[1].c_str(),args[0].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,del,"delete a file","<fileName>")
{
	if (args.size()!=1)
		return false;

	NLMISC::CFile::deleteFile(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,dir,"list files in the current directory","[<wildcard>]")
{
	if (args.size()!=1 && args.size()!=0)
		return false;

	std::string wildcard="*";
	if (args.size()==1)
		wildcard=args[0];

	std::vector<std::string> directories;
	NLMISC::CPath::getPathContent(".",false,true,false,directories);
	for (uint32 i=directories.size();i--;)
	{
		if (!NLMISC::testWildCard(directories[i],wildcard))
		{
			directories[i]=directories.back();
			directories.pop_back();
		}
	}
	std::sort(directories.begin(),directories.end());
	for (uint32 i=0;i<directories.size();++i)
	{
		log.displayNL("%s/",directories[i].c_str());
	}

	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(".",false,false,true,files);
	for (uint32 i=files.size();i--;)
	{
		if (!NLMISC::testWildCard(files[i],wildcard))
		{
			files[i]=files.back();
			files.pop_back();
		}
	}
	std::sort(files.begin(),files.end());
	for (uint32 i=0;i<files.size();++i)
	{
		log.displayNL("%-40s %10d",files[i].c_str(),NLMISC::CFile::getFileSize(files[i]));
	}

	return true;
}

