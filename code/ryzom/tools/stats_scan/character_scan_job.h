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

#ifndef CHARACTER_SCAN_JOB_H
#define CHARACTER_SCAN_JOB_H

//-------------------------------------------------------------------------------------------------
// includes
//-------------------------------------------------------------------------------------------------

#include "stdio.h"

#include "game_share/file_description_container.h"

#include "job_manager.h"
#include "char_info_extractor_factory.h"
#include "char_filter_factory.h"


//-------------------------------------------------------------------------------------------------
// class CCharacterScanJob
//-------------------------------------------------------------------------------------------------

class CCharacterScanJob: public CJobManager::IJob
{
public:
	// inherited virtual interface
	virtual bool finished();
	virtual std::string getShortStatus();
	virtual std::string getStatus();
	virtual void display(NLMISC::CLog* log=NLMISC::InfoLog);
	virtual void update();

public:
	CCharacterScanJob();
	~CCharacterScanJob();

public:
	// interface for initialisation and configuration of the job
	bool charTblAddCol(const std::string& name);
	bool addInfoExtractor(ICharInfoExtractor* infoExtractor);
	bool addFilter(ICharFilter* filter);
	bool addFiles(const CFileDescriptionContainer& fdc);
	bool setOutputPath(const std::string& path);

	// interface for the InfoExtractors to use
	void charTblSetEntry(const std::string& colName,const std::string& value);
	void charTblFlushRow(uint32 account,uint32 slot);
	void freqTblAddEntry(const std::string& tblName, const std::string& key);

private:
	typedef std::vector<NLMISC::CSmartPtr<ICharInfoExtractor> > TInfoExtractors;
	TInfoExtractors _InfoExtractors;

	typedef std::vector<NLMISC::CSmartPtr<ICharFilter> > TFilters;
	TFilters _Filters;

	typedef std::vector<std::string> TFiles;
	TFiles _Files;

	typedef std::map<std::string,uint32> TCharStatsMapTbl;
	typedef std::map<std::string, TCharStatsMapTbl> TCharStatsMap;
	TCharStatsMap _CharStatsMap;

	typedef std::vector<std::string> TTblCols;
	TTblCols _TblCols;

	typedef std::map<std::string,std::string> TCurrentRowEntries;
	TCurrentRowEntries _CurrentRowEntries;

	typedef enum { INIT, WORK, CLOSED, ERROR } TState;
	TState _State;

	std::string _OutputPath;

	FILE* _CharTblFile;
	uint32 _NextFile;
};


//-------------------------------------------------------------------------------------------------
#endif