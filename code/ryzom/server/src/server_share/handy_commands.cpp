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
#include "stdpch.h"
#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/algo.h"
#include "nel/misc/sheet_id.h"
#include "game_share/file_description_container.h"
#include "game_share/persistent_data.h"
#include "game_share/singleton_registry.h"
#include "handy_commands.h"


//-----------------------------------------------------------------------------
// Very handy utility routines for the handy utility commands
//-----------------------------------------------------------------------------

static void readFileList(const NLMISC::CSString& fileListName, CFileDescriptionContainer& result)
{
	// read the file list from disk (the result will be empty if the file list didn't exist)
	NLMISC::CSString fileList;
	fileList.readFromFile(fileListName);

	// split the file list text block into lines
	NLMISC::CVectorSString files;
	fileList.splitLines(files);

	// iterate over the lies in the input file
	for(uint32 i=0;i<files.size();++i)
	{
		// clean lines up, stripping spaces and quotes
		NLMISC::CSString theFile= files[i].strip().unquoteIfQuoted();

		// skip empty lines and comments
		if (theFile.empty() || theFile.left(1)=="#")
			continue;

		// add the file name to the result
		result.addFile(theFile);
	}
}

static void addToFdc(const NLMISC::CSString& filespec, CFileDescriptionContainer& result)
{
	if (filespec.left(1)=="@")
	{
		readFileList(filespec.leftCrop(1),result);
	}
	else
	{
		result.addFileSpec(filespec);
	}
}


//-----------------------------------------------------------------------------
// Handy utility commands - pdr
//-----------------------------------------------------------------------------
// pdrBin2xml <input file name> <output file name>
// pdrXml2bin <input file name> <output file name>
// pdr2xml <input file spec>
// pdr2bin <input file spec>
// pdr2txt <input file spec>


NLMISC_CATEGORISED_COMMAND(utils,pdrBin2xml,"convert a binary pdr file to xml","<input file name> <output file name>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	log.displayNL("Converting to XML : %s => %s",args[0].c_str(),args[1].c_str());

	static CPersistentDataRecord pdr;
	pdr.clear();
	pdr.readFromBinFile(args[0].c_str());
	pdr.writeToTxtFile(args[1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdrXml2bin,"convert a text pdr file to binary","<input file name> <output file name>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	log.displayNL("Converting to Binary : %s => %s",args[0].c_str(),args[1].c_str());

	static CPersistentDataRecord pdr;
	pdr.clear();
	pdr.readFromTxtFile(args[0].c_str());
	pdr.writeToBinFile(args[1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdr2xml,"convert one or more sets of pdr files to xml format","<input file spec>|'@'<listfile> [<input file spec>[...]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	for (uint32 i=0;i<args.size();++i)
	{
		CFileDescriptionContainer fdc;
		addToFdc(args[i],fdc);

		DROP_IF(fdc.empty(),"No files found that match file spec: "<<args[i],continue);

		for (uint32 j=0;j<fdc.size();++j)
		{
			const CFileDescription& fd=fdc[j];
			NLMISC::CSString outputFileName= 
				(fd.FileName.right(4).toLower()==".bin" || fd.FileName.right(4).toLower()==".txt")?
				fd.FileName.rightCrop(4)+".xml": fd.FileName+".xml";

			log.displayNL("Converting to XML : %s => %s",fd.FileName.c_str(),outputFileName.c_str());
			static CPersistentDataRecord	pdr;
			pdr.clear();
			pdr.readFromFile(fd.FileName.c_str());
			pdr.writeToFile(outputFileName.c_str());
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdr2bin,"convert one or more sets of pdr files to binary format","<input file spec>|'@'<listfile> [<input file spec>[...]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	for (uint32 i=0;i<args.size();++i)
	{
		CFileDescriptionContainer fdc;
		addToFdc(args[i],fdc);

		DROP_IF(fdc.empty(),"No files found that match file spec: "<<args[i],continue);

		for (uint32 j=0;j<fdc.size();++j)
		{
			const CFileDescription& fd=fdc[j];
			NLMISC::CSString outputFileName= 
				(fd.FileName.right(4).toLower()==".xml" || fd.FileName.right(4).toLower()==".txt")?
				fd.FileName.rightCrop(4)+".bin": fd.FileName+".bin";

			log.displayNL("Converting to Binary : %s => %s",fd.FileName.c_str(),outputFileName.c_str());
			static CPersistentDataRecord	pdr;
			pdr.clear();
			pdr.readFromFile(fd.FileName.c_str());
			pdr.writeToFile(outputFileName.c_str());
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdr2txt,"convert one or more sets of pdr files to txt (lines) format","<input file spec>|'@'<listfile> [<input file spec>[...]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()<1)
		return false;

	for (uint32 i=0;i<args.size();++i)
	{
		CFileDescriptionContainer fdc;
		addToFdc(args[i],fdc);

		DROP_IF(fdc.empty(),"No files found that match file spec: "<<args[i],continue);

		for (uint32 j=0;j<fdc.size();++j)
		{
			const CFileDescription& fd=fdc[j];
			NLMISC::CSString outputFileName= 
				(fd.FileName.right(4).toLower()==".bin" || fd.FileName.right(4).toLower()==".xml")?
				fd.FileName.rightCrop(4)+".txt": fd.FileName+".txt";

			log.displayNL("Converting to TXT : %s => %s",fd.FileName.c_str(),outputFileName.c_str());
			static CPersistentDataRecord	pdr;
			pdr.clear();
			pdr.readFromFile(fd.FileName.c_str());
			pdr.writeToFile(outputFileName.c_str());
		}
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdrFileCompare,"Compare 2 pdr files","<first file> <second file>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	CPersistentDataRecord pdr0;
	pdr0.readFromFile(args[0].c_str());

	CPersistentDataRecord pdr1;
	pdr1.readFromFile(args[1].c_str());

	log.displayNL("%s : %s / %s", (pdr0==pdr1)?"Files MATCH":"Files DON'T match", args[0].c_str(), args[1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,pdrInfo,"Extract info from pdr file(s)","<input file spec>|'@'<listfile> [<input file spec>[...]] [<output fil name>.csv]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	NLMISC::CSString outputFileName;
	NLMISC::CSString csvTxt;
	uint32 numFileSpecs=(uint32)args.size();

	if (numFileSpecs>0 && NLMISC::CSString(args.back()).right(4)==".csv")
	{
		outputFileName= args.back();
		--numFileSpecs;
		log.displayNL("Extracting PDR info to write to csv file: %s",outputFileName.quoteIfNotQuoted().c_str());
		csvTxt= "FileName,"+ CPersistentDataRecord::getCSVHeaderLine()+"\n";
	}
	else
	{
		log.displayNL("Extracting PDR info (No output file specified so no csv file will be written)",outputFileName.quoteIfNotQuoted().c_str());
	}

	if (numFileSpecs<1)
		return false;

	for (uint32 i=0;i<numFileSpecs;++i)
	{
		CFileDescriptionContainer fdc;
		{
			H_AUTO(pdrInfo_BuildFDC)
			addToFdc(args[i],fdc);
		}

		DROP_IF(fdc.empty(),"No files found that match file spec: "<<args[i],continue);

		for (uint32 j=0;j<fdc.size();++j)
		{
			H_AUTO(pdrInfo_treatFile)
			static CPersistentDataRecord	pdr;
			pdr.clear();

			const CFileDescription& fd=fdc[j];
			log.display("- %s: ",fd.FileName.quoteIfNotQuoted().c_str());

			{
				H_AUTO(pdrInfo_readFromFile)
				pdr.readFromFile(fd.FileName.c_str());
			}
			log.displayNL("%s",pdr.getInfo().c_str());

			if (!outputFileName.empty())
			{
				csvTxt+=NLMISC::CFile::getFilenameWithoutExtension(fd.FileName)+","+pdr.getInfoAsCSV()+"\n";
			}
		}
	}

	if (!outputFileName.empty())
	{
		log.displayNL("Writing file: %s",outputFileName.quoteIfNotQuoted().c_str());
		csvTxt.writeToFile(outputFileName);
	}
	log.displayNL("Info extraction from PDRs finished");

	return true;
}


//-----------------------------------------------------------------------------
// Handy utility commands - file compare
//-----------------------------------------------------------------------------
// quickFileCompare <file0> <file1>
// thoroughFileCompare <file0> <file1> [<max mem footprint>]

NLMISC_CATEGORISED_COMMAND(utils,quickFileCompare,"compare 2 files (by comparing timestamp and size)","<file0> <file1>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	nlinfo("comparing files ...");
	bool result= NLMISC::CFile::quickFileCompare(args[0], args[1]);
	nlinfo("- %s",result?"Same":"Different");

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,thoroughFileCompare,"compare 2 files (by comparing data)","<file0> <file1> [<max mem footprint>]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2 && args.size()!=3)
		return false;

	bool result;
	nlinfo("comparing files ...");

	if (args.size()==3)
	{
		uint32 size;
		NLMISC::fromString(args[2], size);
		if (size<2)
		{
			nlwarning("The third parameter must be a value >= 2 : The following value is not valid: %s",args[2].c_str());
			return true;
		}
		result= NLMISC::CFile::thoroughFileCompare(args[0], args[1], size);
	}
	else
	{
		result= NLMISC::CFile::thoroughFileCompare(args[0], args[1]);
	}

	nlinfo("- %s",result?"Same":"Different");
	return true;
}


//-----------------------------------------------------------------------------
// Handy utility commands - file and directory management
//-----------------------------------------------------------------------------
// cd [<path>]
// md <path>
// copyFile <src> <dest>
// del <fileName>
// dir [<wildcard>]
// deltree <directory>


NLMISC_CATEGORISED_COMMAND(utils,cd,"change directory or display current working directory","[<path>]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=0 && args.size()!=1)
		return false;

	if (args.size()==1)
	{
		NLMISC::CPath::setCurrentPath(args[0].c_str());
	}

	nlinfo("Current directory: %s",NLMISC::CPath::getCurrentPath().c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,md,"create a new directory (or directory tree)","<path>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	NLMISC::CFile::createDirectoryTree(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,copyFile,"copy a file","<src> <dest>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	NLMISC::CFile::copyFile(args[1],args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,del,"delete a file","<fileName>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	NLMISC::CFile::deleteFile(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,dir,"list files in the current directory","[<wildcard>]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1 && args.size()!=0)
		return false;

	std::string wildcard="*";
	if (args.size()==1)
		wildcard=args[0];

	std::vector<std::string> directories;
	NLMISC::CPath::getPathContent(".",false,true,false,directories);
	for (uint32 i=(uint32)directories.size();i--;)
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
		nlinfo("%s/",directories[i].c_str());
	}

	std::vector<std::string> files;
	NLMISC::CPath::getPathContent(".",false,false,true,files);
	for (uint32 i=(uint32)files.size();i--;)
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
		nlinfo("%-40s %10d",files[i].c_str(),NLMISC::CFile::getFileSize(files[i]));
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,deltree,"delete all files matching given spec, recursively","<fileSpec>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	CFileDescriptionContainer tempFiles;
	tempFiles.addFileSpec(args[0],true);
	for (uint32 i=0;i<tempFiles.size();++i)
	{
		NLMISC::CFile::deleteFile(tempFiles[i].FileName);
	}

	return true;
}


//-----------------------------------------------------------------------------
// Handy utility commands - file viewing
//-----------------------------------------------------------------------------
// viewTxtFile <file_name> [<first_line=1>[ <num_lines=100>]]
// viewBinFile <file_name> [<first_offset=0>[ <length=65536>]]

NLMISC_CATEGORISED_COMMAND(utils,viewTxtFile,"view a text file segment","<file_name> [<first_line=1>[ <num_lines=100>]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	// deal with command line arguments
	uint32 firstLine=1;
	uint32 count=100;
	switch (args.size())
	{
	case 3:
		NLMISC::fromString(args[2], count);
		if (count<1||args[2]!=NLMISC::toString("%u",count))
			return false;

	case 2:
		NLMISC::fromString(args[1], firstLine);
		if (firstLine<1||args[1]!=NLMISC::toString("%u",firstLine))
			return false;

	case 1:
		break;
	default:
		return false;
	}

	// read the new file
	NLMISC::CSString fileBody;
	fileBody.readFromFile(args[0]);
	if (fileBody.empty())
	{
		nlwarning("File not found or file empty: %s",args[0].c_str());
		return false;
	}

	// if we have a utf16 file then convert to utf8
	if (fileBody.size()>=2 && ((fileBody[0]==char(0xff) && fileBody[1]==char(0xfe)) || (fileBody[0]==char(0xfe) && fileBody[1]==char(0xff))) )
	{
		nlinfo("Displaying unicode UTF16 text:");
		ucstring ucs;
		ucs.resize((fileBody.size()-2)/2);
		memcpy((char*)&ucs[0],(char*)&fileBody[0],fileBody.size()-2);
		fileBody=ucs.toUtf8();
	}

	// split the new file into lines
	NLMISC::CVectorSString lines;
	fileBody.splitLines(lines);

	// display the lines
	nlinfo("Listing lines %u to %u of %u for file: %s",firstLine,std::min(uint32(firstLine+count-1),(uint32)lines.size()),lines.size(),args[0].c_str());
	for (uint32 i=0;i<count && i+firstLine<=lines.size();++i)
		nlinfo("%6i %s",i+firstLine,lines[i+firstLine-1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,viewBinFile,"view a binary file segment","<file_name> [<first_offset=0>[ <length=65536>]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	// deal with command line arguments
	uint32 start=0;
	uint32 count=65536;
	switch (args.size())
	{
	case 3:
		NLMISC::fromString(args[2], count);
		if (count<1||args[2]!=NLMISC::toString("%u",count))
			return false;

	case 2:
		NLMISC::fromString(args[1], start);
		if (args[1]!=NLMISC::toString("%u",start))
			return false;

	case 1:
		break;
	default:
		return false;
	}

	// read the new file and convert to lines
	NLMISC::CSString fileBody;
	fileBody.readFromFile(args[0]);
	if (fileBody.empty())
	{
		nlwarning("File not found or file empty: %s",args[0].c_str());
		return false;
	}

	// ensure start is valid
	if (start>=fileBody.size())
	{
		nlwarning("first_offset (%u) beyond end of file (%u): %s",start,fileBody.size(),args[0].c_str());
		return false;
	}

	// clamp the value of 'count'
	if (start+count>fileBody.size())
	{
		count= (uint32)fileBody.size()-start;
	}

	// display the data
	uint32 entriesPerLine=20;
	nlinfo("Dumping offset %u to %u of %u for file: %s",start,start+count,fileBody.size(),args[0].c_str());
	for (uint32 i=0;i<(count+15)/entriesPerLine;++i)
	{
		NLMISC::CSString s;

		// generate the address
		s= NLMISC::toString("%10u  ",start+entriesPerLine*i);

		// generate the hex text
		uint32 j=0;
		for (;j<entriesPerLine && entriesPerLine*i+j<count;++j)
			s+=NLMISC::toString("%02X ",(uint8)fileBody[start+entriesPerLine*i+j]);

		// pad out to the start of the ASCII text
		for (;j<entriesPerLine;++j)
			s+="   ";
		s+="  ";

		// generate the ASCII text
		for (j=0;j<entriesPerLine && entriesPerLine*i+j<count;++j)
			s+=NLMISC::CSString::isPrintable(fileBody[start+entriesPerLine*i+j])?fileBody[start+entriesPerLine*i+j]:'.';

		// display the line
		log.displayNL("%s",s.c_str());
	}

	return true;
}

//-----------------------------------------------------------------------------
// Handy utility commands - text editing
//-----------------------------------------------------------------------------
// txtEditAppend <text>
// txtEditCopy <start_line> <end_line> <insert_location>
// txtEditDeleteLines <first_line>[ <last_line>]
// txtEditErase
// txtEditInsert <line_number> <text>
// txtEditList [<first_line=1>[ <num_lines=100>]]
// txtEditMergeFile <insert_location> <file_name> [<first_line> <last_line>]
// txtEditNew <fileName>
// txtEditRead <fileName>
// txtEditSet <line_number> <text>
// txtEditToFile <file name>
// txtEditUnload 
// txtEditWrite 

static NLMISC::CSString TxtEditFileName;
static NLMISC::CVectorSString TxtEditLines;

NLMISC_CATEGORISED_COMMAND(utils,txtEditUnload,"unload the text file in ram","")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	TxtEditFileName.clear();
	TxtEditLines.clear();

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditRead,"load a text file into ram","<fileName>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// make sure there is no file currently loaded
	if (!TxtEditFileName.empty())
	{
		nlwarning("unable to open file '%s' because file '%s' is already open",args[0].c_str(),TxtEditFileName.c_str());
		return true;
	}

	// check that the file exists
	if (!NLMISC::CFile::fileExists(args[0]))
	{
		nlwarning("File Not Found: %s",args[0].c_str());
		return false;
	}

	// read the file and split into lines
	NLMISC::CSString fileBody;
	fileBody.readFromFile(args[0]);
	fileBody.splitLines(TxtEditLines);

	// finish up
	TxtEditFileName=args[0];
	nlinfo("Loaded %u lines from file :%s",TxtEditLines.size(),TxtEditFileName.c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditMergeFile,"load a text file into ram","<insert_location> <file_name> [<first_line> <last_line>]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded - nothing to save");
		return true;
	}

	// make sure there's a valid arg count
	switch (args.size())
	{
	case 4:
	case 2:
		break;
	default:
		return false;
	}

	// get the insert location and make sure it's valid
	uint32 insertPosition;
	NLMISC::fromString(args[0], insertPosition);
	if (insertPosition<1||	args[0]!=NLMISC::toString("%u",insertPosition))	return false;
	if (insertPosition>TxtEditLines.size()+1) { nlwarning("Invalid insert position"); return false; }

	// read the new file and convert to lines
	NLMISC::CVectorSString newLines;
	NLMISC::CSString fileBody;
	fileBody.readFromFile(args[1]);
	fileBody.splitLines(newLines);
	if (fileBody.empty()) { nlwarning("File not found or file empty: %s",args[1].c_str()); return false; }

	if (args.size()==2)
	{
		// we have to merge in the whole file...
		TxtEditLines.insert(TxtEditLines.begin()+(insertPosition-1),newLines.begin(),newLines.end());
		nlinfo("Merged in %u lines from file :%s",newLines.size(),args[1].c_str());
	}
	else
	{
		// we only want part of the new file

		// determine the first and last lines to extract from the new file
		uint32 firstLine, lastLine;
		NLMISC::fromString(args[2], firstLine);
		if (firstLine<1||	args[2]!=NLMISC::toString("%u",firstLine))	return false;
		NLMISC::fromString(args[3], lastLine);
		if (lastLine<1||	args[3]!=NLMISC::toString("%u",lastLine))	return false;

		// make sure the line numbers are valid
		if (firstLine==0||firstLine>lastLine||lastLine>newLines.size())
		{
			nlwarning("invalid line number argument or first line is beyond last line");
			return false;
		}

		// do the merge
		NLMISC::CVectorSString linesToMerge(newLines.begin()+(firstLine-1),newLines.begin()+lastLine);
		TxtEditLines.insert(TxtEditLines.begin()+(insertPosition-1),linesToMerge.begin(),linesToMerge.end());
		nlinfo("Merged in %u lines from file :%s",linesToMerge.size(),args[1].c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditNew,"prepare to edit a new textfile","<fileName>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// make sure there is no file currently loaded
	if (!TxtEditFileName.empty())
	{
		nlwarning("unable to craete new file '%s' because file '%s' is already open",args[0].c_str(),TxtEditFileName.c_str());
		return true;
	}

	// check that the file doesn't exist
	if (NLMISC::CFile::fileExists(args[0]))
	{
		nlwarning("File Already Exists: %s",args[0].c_str());
		return false;
	}

	// finish up
	TxtEditLines.clear();
	TxtEditFileName=args[0];
	nlinfo("Created new empty buffer for file :%s",TxtEditFileName.c_str());
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditWrite,"save a modified text file","")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded - nothing to save");
		return true;
	}

	// join the lines and write to file
	nlinfo("Writing %u lines to file :%s",TxtEditLines.size(),TxtEditFileName.c_str());
	NLMISC::CSString fileBody;
	fileBody.join(TxtEditLines,'\n');
	fileBody.writeToFile(TxtEditFileName);

	// finish up
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditToFile,"save a loaded text file","<file name>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded - nothing to save");
		return true;
	}

	// check that the file doesn't exist
	if (NLMISC::CFile::fileExists(args[0]))
	{
		nlwarning("File Already Exists: %s",args[0].c_str());
		return false;
	}

	// set the new file name
	TxtEditFileName=args[0];

	// join the lines and write to file
	nlinfo("Writing %u lines to file :%s",TxtEditLines.size(),TxtEditFileName.c_str());
	NLMISC::CSString fileBody;
	fileBody.join(TxtEditLines,'\n');
	fileBody.writeToFile(TxtEditFileName);

	// finish up
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditList,"list the lines in a loaded textfile","[<first_line=1>[ <num_lines=100>]]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// deal with command line arguments
	uint32 firstLine=1;
	uint32 count=100;
	switch (args.size())
	{
	case 2:
		NLMISC::fromString(args[1], count);
		if (count<1||args[1]!=NLMISC::toString("%u",count))
			return false;

	case 1:
		NLMISC::fromString(args[0], firstLine);
		if (firstLine<1||args[0]!=NLMISC::toString("%u",firstLine))
			return false;

	case 0:
		break;
	default:
		return false;
	}

	// display the lines
	nlinfo("Listing lines %u to %u of %u for file: %s",firstLine,std::min(uint32(firstLine+count-1),(uint32)TxtEditLines.size()),TxtEditLines.size(),TxtEditFileName.c_str());
	for (uint32 i=0;i<count && i+firstLine<=TxtEditLines.size();++i)
		nlinfo("%6i %s",i+firstLine,TxtEditLines[i+firstLine-1].c_str());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditDeleteLines,"delete one or more lines in a loaded textfile","<first_line>[ <last_line>]")
{
	NLMISC::CNLLogOverride logOverride(&log);

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// deal with command line arguments
	uint32 firstLine=0;
	uint32 lastLine=0;
	switch (args.size())
	{
	case 2:
		NLMISC::fromString(args[1], lastLine);
		if (lastLine<1||args[1]!=NLMISC::toString("%u",lastLine))
			return false;

	case 1:
		NLMISC::fromString(args[0], firstLine);
		if (firstLine<1||args[0]!=NLMISC::toString("%u",firstLine))
			return false;
		break;

	default:
		return false;
	}

	if (lastLine==0)
		lastLine= firstLine;

	if (lastLine<firstLine)
	{
		nlwarning("last_line (%u) must be >= first_line (%u)",lastLine,firstLine);
		return false;
	}

	// ensure that the lines to be deleted fall within the file
	if (lastLine>TxtEditLines.size())
	{
		nlwarning("last_line (%u) must be <= num lines (%u) in file: %s",lastLine,TxtEditLines.size(),args[0].c_str());
		return false;
	}

	// delete the lines
	nlinfo("Deleting lines %u to %u of %u for file: %s",firstLine,lastLine,TxtEditLines.size(),args[0].c_str());
	TxtEditLines.erase(TxtEditLines.begin()+firstLine-1,TxtEditLines.begin()+lastLine);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditErase,"erase all of the current contents of the currently loaded file","")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=0)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// do the work...
	TxtEditLines.clear();

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditAppend,"append a line of text to a loaded textfile","<text>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=1)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	TxtEditLines.push_back(args[0]);

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditCopy,"duplicate a segment of the text file and insert it at the given location","<start_line> <end_line> <insert_location>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=3)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// extract numeric values for args and verify theri validity
	uint32 firstLine, lastLine, insertPosition;
	NLMISC::fromString(args[0], firstLine);
	if (firstLine<1||		args[0]!=NLMISC::toString("%u",firstLine))		return false;
	NLMISC::fromString(args[1], lastLine);
	if (lastLine<1||		args[1]!=NLMISC::toString("%u",lastLine))		return false;
	NLMISC::fromString(args[2], insertPosition);
	if (insertPosition<1||	args[2]!=NLMISC::toString("%u",insertPosition))	return false;

	// make sure the line numbers are valid
	if (firstLine>lastLine||lastLine>TxtEditLines.size()||insertPosition>TxtEditLines.size()+1)
	{
		nlwarning("invalid line number argument or first line is beyond last line");
		return false;
	}

	// duplicate the lines in question and insert them
	NLMISC::CVectorSString newLines(TxtEditLines.begin()+firstLine-1,TxtEditLines.begin()+lastLine);
	TxtEditLines.insert(TxtEditLines.begin()+(insertPosition-1),newLines.begin(),newLines.end());

	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditInsert,"insert a line into a loaded text file","<line_number> <text>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// extract the line number
	uint32 lineNumber;
	NLMISC::fromString(args[0], lineNumber);
	if (lineNumber<1||args[0]!=NLMISC::toString("%u",lineNumber))
		return false;

	// deal with special case of insert at end of buffer
	if (lineNumber==TxtEditLines.size()+1)
	{
		TxtEditLines.push_back(args[1]);
		return true;
	}

	// make sure the line number falls within the file
	if (lineNumber>TxtEditLines.size())
	{
		nlwarning("line number (%u) must be less than or equal to num lines in file (%u)",lineNumber,TxtEditLines.size());
		return false;
	}

	TxtEditLines.insert(TxtEditLines.begin()+(lineNumber-1),args[1]);
	return true;
}

NLMISC_CATEGORISED_COMMAND(utils,txtEditSet,"change a line in a loaded text file","<line_number> <text>")
{
	NLMISC::CNLLogOverride logOverride(&log);

	if (args.size()!=2)
		return false;

	// make sure there is a file currently loaded
	if (TxtEditFileName.empty())
	{
		nlwarning("no text file currently loaded");
		return true;
	}

	// extract the line number
	uint32 lineNumber;
	NLMISC::fromString(args[0], lineNumber);
	if (lineNumber<1||args[0]!=NLMISC::toString("%u",lineNumber))
		return false;

	// make sure the line number falls within the file
	if (lineNumber>TxtEditLines.size())
	{
		nlwarning("line number (%u) must be less than or equal to num lines in file (%u)",lineNumber,TxtEditLines.size());
		return false;
	}

	TxtEditLines[lineNumber-1]= args[1];
	return true;
}


//-----------------------------------------------------------------------------
// init the sheetid.bin file for use by PDR
//-----------------------------------------------------------------------------

class CForSheetId: public IServiceSingleton
{
public:
	void init()
	{
//		NLMISC::CSheetId::init(false);
	}
};

static CForSheetId ForSheetId;


//-------------------------------------------------------------------------------------------------
// Fake variable to force a link to the handy_commands module
//-------------------------------------------------------------------------------------------------

CHandyCommandIncluderClass::CHandyCommandIncluderClass()
{
	static bool firstTime= true;

	if (!firstTime)
		return;

//	nldebug("Activating Handy Commands");
	firstTime= false;
}


//-----------------------------------------------------------------------------
