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

#ifndef STAT_GUILD_SCAN_JOB_H
#define STAT_GUILD_SCAN_JOB_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "game_share/file_description_container.h"

#include "stat_guild_container.h"
#include "stat_job_manager.h"


//-------------------------------------------------------------------------------------------------
// class CGuildScanJob
//-------------------------------------------------------------------------------------------------

class CGuildScanJob: public CJobManager::IJob
{
public:
	// inherited virtual interface
	virtual void start();
	virtual bool finished();
	virtual std::string getShortStatus();
	virtual std::string getStatus();
	virtual void display(NLMISC::CLog* log=NLMISC::InfoLog);
	virtual void update();

public:
	// ctor / dtor
	CGuildScanJob(CStatGuildContainer* guildContainer,const NLMISC::CSString& outputRoot="");
	~CGuildScanJob();

public:
	// interface for initialisation and configuration of the job
	void addFileSpec(const NLMISC::CSString& fileSpec,bool recursive=false);

private:
	// private data - internal to job
	enum TState { INIT, WORK, CLOSED, ERROR };
	TState _State;
	uint32 _NextFile;
	CFileDescriptionContainer _Files;

	// file specs container
	typedef std::vector<NLMISC::CSString> TFileSpecs;
	TFileSpecs _RecursiveFileSpecs;
	TFileSpecs _NonRecursiveFileSpecs;

	// pointer to container that the job is supposed to be filling - if NULL job will abort
	NLMISC::CRefPtr<CStatGuildContainer> _GuildContainer;

	// the location for the csv files to be written to
	// - if empty, no csv files are written
	// - for values like './abc/def/' the output files take the form: './abc/def/xxx.csv'
	// - for values like './abc/def' the output files take the form: './abc/def_xxx.csv'
	NLMISC::CSString _OutputRoot;
};


//-------------------------------------------------------------------------------------------------
#endif
