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

//nel
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

// game share
#include "game_share/utils.h"
#include "game_share/file_description_container.h"
#include "game_share/singleton_registry.h"
#include "game_share/persistent_data.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;


//-----------------------------------------------------------------------------
// Variables
//-----------------------------------------------------------------------------

//NLMISC::CVariable<string>	Bla("ShardMerge", "Bla", "Directory containing event tools", string("./events/tools/"), 0, true);


//-----------------------------------------------------------------------------
// CContestCtrlScript Commands
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(ShardMerge,mergeGuildIdFix,"set the guild ids in the pdr records from the values in their file names","<filespec>")
{
	if (args.size()!=1)
		return false;

	CFileDescriptionContainer fdc;
	fdc.addFileSpec(args[0]);
	for (uint32 i=0;i<fdc.size();++i)
	{
		uint32 id= NLMISC::CSString(NLMISC::CFile::getFilenameWithoutExtension(fdc[i].FileName)).splitFrom('_').atoui();
		nlinfo("Setting id for guild: %s to: %d",fdc[i].FileName.c_str(),id);

		// read the file
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromFile(fdc[i].FileName.c_str());

		// convert to XML
		NLMISC::CSString inputString;
		pdr.toXML(inputString);

		NLMISC::CSString resultString;
		resultString+=	inputString.splitTo("<Id type=\"UINT32\" value=\"");
		resultString+=	"<Id type=\"UINT32\" value=\"";
		resultString+=	NLMISC::toString("%u",id);
		resultString+=	"\"/>";
		resultString+=	inputString.splitFrom("<Id type=\"UINT32\" value=\"").splitFrom("\"/>");

		// convert back to a pdr
		pdr.clear();
		pdr.fromXML(resultString);

		// write the file
		pdr.writeToFile((fdc[i].FileName+".new.xml").c_str());
		pdr.writeToFile((fdc[i].FileName+".new.bin").c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardMerge,mergeChangeCharacterNames,"change names of characters to thos listed in charcter_names.txt file","<save root directory>")
{
	DROP_IF(args.size()!=1,"Syntax error: Requires an argument",return false);
	DROP_IF(!NLMISC::CFile::fileExists(args[0]+"/character_names.txt"),"File not found: "+args[0]+"/character_names.txt",return false);

	NLMISC::CSString fileContent;
	fileContent.readFromFile(args[0]+"/character_names.txt");
	NLMISC::CVectorSString fileLines;
	fileContent.splitLines(fileLines);

	for (uint32 i=0;i<fileLines.size();++i)
	{
		NLMISC::CSString name= fileLines[i].word(0);
		uint32 account= fileLines[i].word(1).atoui();
		uint32 slot= fileLines[i].word(2).atoui();
		nlinfo("Setting name for character: %d_%d to: %s",account,slot,name.c_str());

		// read the file
		NLMISC::CSString fileName= args[0]+NLMISC::toString("/characters/account_%d_%d_pdr.bin",account,slot);
		DROP_IF(!NLMISC::CFile::fileExists(fileName),"Skipping inexistant file: "+fileName,continue);
		static CPersistentDataRecord	pdr;
		pdr.clear();
		pdr.readFromFile(fileName.c_str());

		// convert to XML
		NLMISC::CSString inputString;
		pdr.toXML(inputString);

		NLMISC::CSString resultString;
		resultString+=	inputString.splitTo("<_Name type=\"STRING\" value=\"");
		resultString+=	"<_Name type=\"STRING\" value=\"";
		resultString+=	name;
		resultString+=	"\"/>";
		resultString+=	inputString.splitFrom("<_Name type=\"STRING\" value=\"").splitFrom("\"/>");

		// convert back to a pdr
		pdr.clear();
		pdr.fromXML(resultString);

		// write the file
		pdr.writeToFile((fileName+".new.xml").c_str());
		pdr.writeToFile((fileName+".new.bin").c_str());
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(ShardMerge,mergeBuildCharacterNamesFromCSV,"build a character names file from a CSV file containing columns: account, accountSlot, name","<csv file><character names file>")
{
	if (args.size()!=2)
		return false;

	// read the CSV file
	NLMISC::CSString input;
	input.readFromFile(args[0].c_str());

	// convert CSV input into lines
	NLMISC::CVectorSString lines;
	input.splitLines(lines);
	DROP_IF(lines.empty(),"Failed to extract meaningful data from input file: "+args[0],return false);

	// split the first line into column titles
	NLMISC::CVectorSString columns;
	lines[0].splitByOneOfSeparators(",;",columns);

	// run through the column titles and identify the columns that interest us
	uint32 accountIdx=~0u;
	uint32 accountSlotIdx=~0u;
	uint32 nameIdx=~0u;
	for (uint32 i=0;i<columns.size();++i)
	{
		if (columns[i]=="account")		accountIdx=	i;
		if (columns[i]=="accountSlot")	accountSlotIdx=	i;
		if (columns[i]=="name")			nameIdx=	i;
	}
	DROP_IF(accountIdx==~0u||accountSlotIdx==~0u||nameIdx==~0u,"Failed to identify columns (account,accountSlot,name) in input file: "+args[0],return false);

	// compose the output data by running through the lines of the input data extracting the required columns
	NLMISC::CSString result;
	for (uint32 i=1;i<lines.size();++i)
	{
		NLMISC::CVectorSString fields;
		lines[i].splitByOneOfSeparators(",;",fields);
		BOMB_IF(fields.size()<std::max(nameIdx,std::max(accountIdx,accountSlotIdx)),
			"Skipping line from input file '"+args[0]+"' due to insufficient fields: "+lines[i],continue);
		result+=fields[nameIdx]+" "+fields[accountIdx]+" "+fields[accountSlotIdx]+"\n";
	}

	// write the output file
	BOMB_IF(result.empty(),"Failed to extract any usable data from input file: "+args[0],return false);
	result.writeToFile(args[1]);

	return true;
}


NLMISC_CATEGORISED_COMMAND(ShardMerge,mergeIdentifyOverlappingCharacters,"identify overlapping characters from a set of character_name.txt type files","<first file>[<second file>[...]]")
{
	if (args.size()<1)
		return false;

	// setup maps of all account ids and character names accross all input files
	std::map<NLMISC::CSString,NLMISC::CSString> accounts;	// <account_name,input_file+":"+character name>
	std::map<NLMISC::CSString,NLMISC::CSString> names;		// <character name,input_file+":"+account_name>

	// for each input file: ...
	for (uint32 i=0;i<args.size();++i)
	{
		// read the input file
		NLMISC::CSString input;
		input.readFromFile(args[i].c_str());
		DROP_IF(input.empty(),"Failed to read input file: "+args[i],return false);

		// convert to lines
		NLMISC::CVectorSString lines;
		input.splitLines(lines);

		// for each line...
		for (uint32 j=0;j<lines.size();++j)
		{
			// extract interesting info from the line
			DROP_IF(lines[j].countWords()!=3,"Invalid line in file '"+args[i]+"': "+lines[j],return false);
			const NLMISC::CSString name= lines[i].word(0);
			const NLMISC::CSString account= lines[j].word(1)+"_"+lines[j].word(2);

			// add entry to accounts map
			if (accounts.find(account)==accounts.end()) 
				accounts[account]=args[i]+":"+name;
			else
				accounts[account]+=","+args[i]+":"+name;

			// add entry to character_names map
			if (names.find(name)==names.end()) 
				names[name]=args[i]+":"+account;
			else
				names[name]+=","+args[i]+":"+account;
		}
	}

	// setup a string to record duplicate names into
	NLMISC::CSString output;

	// run through the accounts map looking for duplicate entries
	for (std::map<NLMISC::CSString,NLMISC::CSString>::iterator it= accounts.begin();it!=accounts.end();++it)
	{
		if ((*it).second.contains(','))
		{
			output+="Account "+(*it).first+":"+(*it).second+"\n";
			nlinfo("Duplicate account: %s @ %s",(*it).first.c_str(),(*it).second.c_str());
		}
	}

	// run through the names map looking for duplicate entries
	for (std::map<NLMISC::CSString,NLMISC::CSString>::iterator it= names.begin();it!=names.end();++it)
	{
		if ((*it).second.contains(','))
		{
			output+="Name "+(*it).first+":"+(*it).second+"\n";
			nlinfo("Duplicate character name: %s @ %s",(*it).first.c_str(),(*it).second.c_str());
		}
	}

	// if no duplicates were found then drop out
	DROP_IF(output.empty(),"No duplicates found!",return true);

	// dump the output to disk
	NLMISC::CSString outputFileName= args[0]+".err";
	nlinfo("Writintg duplicate list to output file: %s",outputFileName.c_str());
	output.writeToFile(outputFileName);

	return true;
}


//-----------------------------------------------------------------------------
