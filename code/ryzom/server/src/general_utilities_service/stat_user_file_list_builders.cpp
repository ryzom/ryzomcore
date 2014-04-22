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

#include <time.h>
#include "game_share/utils.h"
#include "stat_file_list_builder_factory.h"
#include "stat_globals.h"


//-------------------------------------------------------------------------------------------------
// namespaces
//-------------------------------------------------------------------------------------------------

using namespace std;
using namespace NLMISC;

//-------------------------------------------------------------------------------------------------
// FILE_LIST_BUILDER Implementations
//-------------------------------------------------------------------------------------------------

FILE_LIST_BUILDER(AddFiles,"<filespec> [<filespec>[...]]")
{
	if (_RawArgs.strip().empty())
		return false;

	CVectorSString fspecs;
	_RawArgs.strip().splitByOneOfSeparators(" \t",fspecs);

	for (uint32 i=0;i<fspecs.size();++i)
		fdc.addFileSpec(STAT_GLOBALS::getInputFilePath(fspecs[i].unquoteIfQuoted()));

	return true;
}

FILE_LIST_BUILDER(AddFilesRecurse,"<filespec> [<filespec>[...]]")
{
	if (_RawArgs.strip().empty())
		return false;

	CVectorSString fspecs;
	_RawArgs.strip().splitByOneOfSeparators(" \t",fspecs);

	for (uint32 i=0;i<fspecs.size();++i)
		fdc.addFileSpec(STAT_GLOBALS::getInputFilePath(fspecs[i].unquoteIfQuoted()),true);

	return true;
}

FILE_LIST_BUILDER(MinFileSize,"<min_file_size>")
{
	uint32 minSize= _RawArgs.atoi();
	if (minSize<1)
	{
		nlwarning("Bad minimum file size: %s",_RawArgs.c_str());
		return false;
	}

	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		if (fdc[i].FileSize<minSize)
			fdc.removeFile(i);
	}

	return true;
}

FILE_LIST_BUILDER(MaxFileSize,"<max_file_size>")
{
	uint32 maxSize= _RawArgs.atoi();
	if (maxSize<1)
	{
		nlwarning("Bad maximum file size: %s",_RawArgs.c_str());
		return false;
	}

	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		if (fdc[i].FileSize>maxSize)
			fdc.removeFile(i);
	}

	return true;
}

FILE_LIST_BUILDER(FileAge,"<min> [<max>]")
{
	// the args here are min and max numbers of days for file ages

	CVectorSString args;
	_RawArgs.strip().splitByOneOfSeparators(" \t",args);

	uint32 min=0;
	uint32 max=~0u;
	switch (args.size())
	{
	case 2:	// deal with explicit 'max' arg (if there is one)
		max= args[1].atoi();
		DROP_IF(args[1]!=NLMISC::toString("%u",max),"Invalid arg for FileAge: "+_RawArgs,return false);
		// drop through to deal with the 'min' argument

	case 1:	// deal with 'min' arg
		min= args[0].atoi();
		DROP_IF(args[0]!=NLMISC::toString("%u",min),"Invalid arg for FileAge: "+_RawArgs,return false);
		break;

	default:	// no args or too many args
		DROP("Bad number of Arguments found for FileAge: "+_RawArgs,return false);
	}

	// calculate the timestamp value of midnight today
	time_t theTime;
	time(&theTime);
	uint32 dateToday= theTime/(24*64*60)*(24*64*60);

	// calculate the maximum and minimum timestamp values corresponding to the dates that we've been given
	uint32 minFileTime= (max==~0u)? 0: dateToday-(max*24*60*60);
	uint32 maxFileTime= dateToday-(min*24*60*60);

	// remove files that don't meet our time criteria
	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		if (fdc[i].FileTimeStamp>=maxFileTime || fdc[i].FileTimeStamp<minFileTime)
			fdc.removeFile(i);
	}

	// display a little log...
	nlinfo("FileAge %s: matched %u of %u files",_RawArgs.c_str(),fdc.size(),ttlNumFiles);

	return true;
}

FILE_LIST_BUILDER(Since,"<dd>/<mm>/<yyyy> - include only files modified on or after given date")
{
	// setup a new time structure, extracting the day, month and year values from the argument string
	struct tm tmstruct;
	NLMISC::CSString txt= _RawArgs.strip();
	tmstruct.tm_mday	= txt.splitTo('/',true,true).atoi();
	tmstruct.tm_mon		= txt.splitTo('/',true,true).atoi() -1;
	tmstruct.tm_year	= txt.atoi();
	if (tmstruct.tm_year<100)
		tmstruct.tm_year= ((tmstruct.tm_year+30)%100)+1970;

	// make sure the day month and year are valid
	DROP_IF(tmstruct.tm_year<1970 || tmstruct.tm_year>2100,"FILE_LIST_BUILDER 'Since' invalid year: "+_RawArgs,return false);
	DROP_IF(tmstruct.tm_mon<1 || tmstruct.tm_mon>12,"FILE_LIST_BUILDER 'Since' invalid month: "+_RawArgs,return false);
	DROP_IF(tmstruct.tm_mday<1 || tmstruct.tm_mday>31,"FILE_LIST_BUILDER 'Since' invalid day: "+_RawArgs,return false);

	// complete initialisation of tm struct (and map year into range from 1970 up
	tmstruct.tm_year	-= 1900;
	tmstruct.tm_wday	= 0;
	tmstruct.tm_yday	= 0;
	tmstruct.tm_isdst	= 0;

	// build a time_t value for the start of the day
	tmstruct.tm_sec		= 0;
	tmstruct.tm_min		= 0;
	tmstruct.tm_hour	= 0;
	uint32 minFileTime= (uint32)mktime( &tmstruct );

	// make sure the generated time value is valid
	DROP_IF(minFileTime==~0u,"FILE_LIST_BUILDER 'Since' mktime() failed: "+_RawArgs,return false);

	// remove files that don't meet our time criteria
	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		if (fdc[i].FileTimeStamp<(uint32)minFileTime)
			fdc.removeFile(i);
	}

	// display a little log...
	nlinfo("Since %s: matched %u of %u files",_RawArgs.c_str(),fdc.size(),ttlNumFiles);

	return true;
}

FILE_LIST_BUILDER(Before,"<dd>/<mm>/<yyyy> - include only files modified strictly before given date")
{
	// setup a new time structure, extracting the day, month and year values from the argument string
	struct tm tmstruct;
	NLMISC::CSString txt= _RawArgs.strip();
	tmstruct.tm_mday	= txt.splitTo('/',true,true).atoi();
	tmstruct.tm_mon		= txt.splitTo('/',true,true).atoi() -1;
	tmstruct.tm_year	= txt.atoi();
	if (tmstruct.tm_year<100)
		tmstruct.tm_year= ((tmstruct.tm_year+30)%100)+1970;

	// make sure the day month and year are valid
	DROP_IF(tmstruct.tm_year<1970 || tmstruct.tm_year>2100,"FILE_LIST_BUILDER 'Before' invalid year: "+_RawArgs,return false);
	DROP_IF(tmstruct.tm_mon<1 || tmstruct.tm_mon>12,"FILE_LIST_BUILDER 'Before' invalid month: "+_RawArgs,return false);
	DROP_IF(tmstruct.tm_mday<1 || tmstruct.tm_mday>31,"FILE_LIST_BUILDER 'Before' invalid day: "+_RawArgs,return false);

	// complete initialisation of tm struct (and map year into range from 1970 up
	tmstruct.tm_year	-= 1900;
	tmstruct.tm_wday	= 0;
	tmstruct.tm_yday	= 0;
	tmstruct.tm_isdst	= 0;

	// build a time_t value for the end of the day
	tmstruct.tm_sec		= 0;
	tmstruct.tm_min		= 0;
	tmstruct.tm_hour	= 0;
	uint32 maxFileTime= (uint32)mktime( &tmstruct );

	// make sure the generated time value is valid
	DROP_IF(maxFileTime==~0u,"FILE_LIST_BUILDER 'Before' mktime() failed: "+_RawArgs,return false);

	// remove files that don't meet our time criteria
	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		if (fdc[i].FileTimeStamp>=(uint32)maxFileTime)
			fdc.removeFile(i);
	}

	// display a little log...
	nlinfo("Before %s: matched %u of %u files",_RawArgs.c_str(),fdc.size(),ttlNumFiles);

	return true;
}


FILE_LIST_BUILDER(NewestFile,"remove all but the latest file for each account (assuming files are of format .*_<nnnn>_.*)")
{
	// array of account ids corresponding to entries in fdc (before removing entries)
	typedef std::vector<uint32> TAccountIds;
	TAccountIds accountIds(fdc.size());

	// map of account number to most recent time stamp for files belonging to the account
	typedef std::map<uint32,uint32> TBestTimes;
	TBestTimes bestTimes;

	// generate the account numbers corresponding to the file names in the fdc
	// also initialise bestDate map entries for all of the account numbers found
	for (uint32 i=fdc.size();i--;)
	{
		NLMISC::CSString s= NLMISC::CFile::getFilename(fdc[i].FileName);
		uint32 accountId;
		do
		{
			DROP_IF(s.empty(),"No account number found in file: "+fdc[i].FileName,accountIds[i]=0;continue);
			accountId=s.strtok("_.").atoui();
		}
		while (accountId==0);
		accountIds[i]= accountId;
		bestTimes[accountIds[i]]=0;
	}
	
	// setup the most recent files times for each account
	for (uint32 i=fdc.size();i--;)
	{
		uint32 accountId= accountIds[i];
		uint32& bestTime= bestTimes[accountId];
		uint32 timeStamp= fdc[i].FileTimeStamp;
		bestTime=std::max(timeStamp,bestTime);
	}

	// remove files that are not the most recent ones
	uint32 ttlNumFiles= fdc.size();
	for (uint32 i=ttlNumFiles;i--;)
	{
		uint32 accountId= accountIds[i];
		uint32& bestTime= bestTimes[accountId];
		uint32 timeStamp= fdc[i].FileTimeStamp;
		if (bestTime!=timeStamp || accountId==0)
		{
			fdc.removeFile(i);
			continue;
		}
		// this file matches the timestamp, so increase the time stamp to prevent matching with multiple identicle files
		++bestTime;
	}

	// display a little log...
	nlinfo("NewestFile: matched %u of %u files",fdc.size(),ttlNumFiles);

	return true;
}

