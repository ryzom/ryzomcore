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

#include "stats_guild_scan_job.h"


//-------------------------------------------------------------------------------------------------
// methods CGuildScanJob
//-------------------------------------------------------------------------------------------------

CGuildScanJob::CGuildScanJob(CStatGuildContainer* guildContainer,const NLMISC::CSString& outputRoot)
{
	// start by initialising simple properties
	_State=INIT;
	_NextFile=0;
	_GuildContainer=guildContainer;
	_OutputRoot=outputRoot;
}

CGuildScanJob::~CGuildScanJob()
{
	if (_State!=ERROR)
		_State=CLOSED;

	// end the scan and flush the output files to disk (if any)
	if (_GuildContainer!=NULL)
	{
		_GuildContainer->endScan();

		if(!_OutputRoot.empty())
		{
			NLMISC::CSString outputRoot= _OutputRoot;
			if (outputRoot.right(1)!="/")
				outputRoot+='_';
			_GuildContainer->writeMemberListFile(outputRoot+"members.csv");
			_GuildContainer->writeInventoryFile(outputRoot+"inventories.csv");
			_GuildContainer->writeMiscDataFile(outputRoot+"info.csv");
		}
	}
}

void CGuildScanJob::start()
{
	// setup the file list before the job begins
	_Files.clear();

	for (uint32 i=0;i<_NonRecursiveFileSpecs.size();++i)
	{
		nlinfo("Scanning for input files: %s ...",_NonRecursiveFileSpecs[i].c_str());
		_Files.addFileSpec(_NonRecursiveFileSpecs[i],false);
	}

	for (uint32 i=0;i<_RecursiveFileSpecs.size();++i)
	{
		nlinfo("Scanning for input files (recursive): %s ...",_RecursiveFileSpecs[i].c_str());
		_Files.addFileSpec(_RecursiveFileSpecs[i],true);
	}

	nlinfo("Input file scan completed: %d files found ... %s",_Files.size(),_Files.empty()?"Nothing to do!":"starting work");

	// set the state to 'WORK' meaning we've finished init and now its ok to do some work
	_State=WORK;

	// setup the target guild container for the new scan
	if (_GuildContainer!=NULL)
	{
		_GuildContainer->startScan();
	}
}

bool CGuildScanJob::finished()
{
	return (_NextFile>= _Files.size()) || (_GuildContainer==NULL);
}

std::string CGuildScanJob::getShortStatus()
{
	return NLMISC::toString("GuildFiles %d/%d",_NextFile,_Files.size());
}

std::string CGuildScanJob::getStatus()
{
	return getShortStatus();
}

void CGuildScanJob::display(NLMISC::CLog* log)
{
	log->displayNL("%s",getStatus().c_str());
}

void CGuildScanJob::update()
{
	// if nothing left to do then give up
	if (finished())
		return;

	// treat the next file in the list
	NLMISC::CSString fileName=_Files[_NextFile].FileName;

	// read the file contents
	static CPersistentDataRecord	fileContent;
	fileContent.clear();
	fileContent.readFromFile(fileName.c_str());

	// add the result to the target guild container
	if (_GuildContainer!=NULL)
	{
		_GuildContainer->addGuildFile(fileName,fileContent);
	}

	// move counter on to next file
	++_NextFile;
}

void CGuildScanJob::addFileSpec(const NLMISC::CSString& fileSpec,bool recursive)
{
	if(recursive)
	{
		_RecursiveFileSpecs.push_back(fileSpec);
	}
	else
	{
		_NonRecursiveFileSpecs.push_back(fileSpec);
	}
}


//-------------------------------------------------------------------------------------------------
