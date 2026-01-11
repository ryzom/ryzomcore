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

#include "nel/misc/path.h"
#include "nel/misc/command.h"
#include "nel/misc/variable.h"

#include "stat_guild_container.h"
#include "stats_guild_scan_job.h"
#include "stat_globals.h"


//-----------------------------------------------------------------------------
// Namespaces
//-----------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;


//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------

NLMISC::CSmartPtr<CStatGuildContainer> GuildContainer;


//-----------------------------------------------------------------------------
// Commands - character and account names
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,loadCharacterNames,"load the character_names.txt file for a given shard","[<file name>] [<shard>]")
{
	CSString shardName= "default";
	CSString fileName= "character_names.txt";

	switch(args.size())
	{
	case 2: shardName=args[1];
	case 1: fileName=args[0];
	case 0: break;
	default: return false;
	}

	CSString fileContent;
	fileContent.readFromFile(fileName);
	DROP_IF(fileContent.empty(),"Failed to read character names file: "+fileName,return true);

	CVectorSString lines;
	fileContent.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
	{
		// ignore blank lines
		if (lines[i].strip().empty())
			continue;

		// break line into fields
		CSString name= lines[i].word(0);
		uint32 account= lines[i].word(1).atoui();
		uint32 slot= lines[i].word(2).atoi();
		DROP_IF(lines[i].countWords()!=3 || account==0 || slot>15,"Skipping bad character name mapping: "+lines[i],continue);

		// add the new mapping to the character name singleton
		STAT_GLOBALS::addCharacterNameMapping(shardName,account,slot,name);
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,loadAccountNames,"load an account_names.txt file","[<file name>]")
{
	CSString fileName= "account_names.txt";

	switch(args.size())
	{
	case 1: fileName=args[0];
	case 0: break;
	default: return false;
	}

	CSString fileContent;
	fileContent.readFromFile(fileName);
	DROP_IF(fileContent.empty(),"Failed to read account names file: "+fileName,return true);

	CVectorSString lines;
	fileContent.splitLines(lines);
	for (uint32 i=0;i<lines.size();++i)
	{
		// ignore blank lines
		if (lines[i].strip().empty())
			continue;

		// break line into fields
		CSString name= lines[i].word(0);
		uint32 account= lines[i].word(1).atoui();
		DROP_IF(lines[i].countWords()!=2 || account==0,"Skipping bad account name mapping: "+lines[i],continue);

		// add the new mapping to the character name singleton
		STAT_GLOBALS::addAccountNameMapping(account,name);
	}

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,loadSheetNames,"load a txt file containing sheet names","<file name>")
{
	if (args.size()!=0)
		return false;

	nlwarning("*** todo ***");

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,clearCharacterNames,"load the character_names.txt file for a given shard","[<shard>] <file name>")
{
	switch(args.size())
	{
	case 0: STAT_GLOBALS::clearAllCharacterNames(); return true;
	case 1: STAT_GLOBALS::clearCharacterNames(args[0]); return true;
	}
	return false;
}

NLMISC_CATEGORISED_COMMAND(Stats,clearAccountNames,"load an account_names.txt file","<file name>")
{
	if (args.size()!=0)
		return false;

	STAT_GLOBALS::clearAccountNames();

	return true;
}

NLMISC_CATEGORISED_COMMAND(Stats,clearSheetNames,"load a txt file containing sheet names","<file name>")
{
	if (args.size()!=0)
		return false;

	STAT_GLOBALS::clearSheetNames();

	return true;
}


//-----------------------------------------------------------------------------
// Commands - guild scanner
//-----------------------------------------------------------------------------

NLMISC_CATEGORISED_COMMAND(Stats,guildScan,"scan a set of guild files","<file specs>[ <file specs>[...]]")
{
	CNLSmartLogOverride logOverride(&log);

	if (args.size()==0)
		return false;

	// create a new job
	GuildContainer= new CStatGuildContainer;
	NLMISC::CSmartPtr<CGuildScanJob> theJob= new CGuildScanJob(GuildContainer,STAT_GLOBALS::getOutputFilePath());

	// setup the file specs for scanning
	for (uint32 i=0;i<args.size();++i)
	{
		theJob->addFileSpec(args[i]);
	}

	// start the job running
	CJobManager::getInstance()->addJob(&*theJob);


	return true;
}


//-----------------------------------------------------------------------------
