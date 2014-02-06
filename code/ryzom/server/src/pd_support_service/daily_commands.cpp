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


#include <time.h>
#include "nel/misc/types_nl.h"
#include "nel/misc/file.h"
#include "nel/misc/path.h"
#include "nel/misc/sstring.h"
#include "nel/net/service.h"
#include "game_share/utils.h"
#include "game_share/singleton_registry.h"
#include "stat_job_manager.h"

NLMISC::CVariable<std::string> DailyActivityLogFileName("scheduler", "DailyActivityLogFileName", "log file name for daily activity logs", "pdss_daily_activity.log", 0, true);
NLMISC::CVariable<uint32> DailyTimeInterval("scheduler", "DailyTimeInterval", "number of seconds in a day", 24*60*60, 0, true);
NLMISC::CVariable<uint32> DailyStartTime("scheduler", "DailyStartTime", "number of seconds into the day when daily tasks should be launched", 8*60*60, 0, true);

class CDailyTaskScheduler: public IServiceSingleton
{
public:
	void serviceUpdate()
	{
		static uint32 jobsRemaining=0;

		// if there are jobs still running then just chart progress
		if (jobsRemaining!=0)
		{
			if (CJobManager::getInstance()->getNumJobs() < jobsRemaining)
			{
				nlinfo("JobManager state: %s",CJobManager::getInstance()->getStatus().c_str());
				jobsRemaining= CJobManager::getInstance()->getNumJobs();
			}
			return;
		}
		
		// get the start time
		time_t startTime;
		time( &startTime );

		// if we're too early in the morning then return
		if ( uint32(startTime%DailyTimeInterval.get()) <= DailyStartTime )
			return;

		// setup the lastTime corresponding to the last time we ran
		static uint32 lastTime=0;
		if (lastTime==0 && NLMISC::CFile::fileExists(DailyActivityLogFileName))
		{
			lastTime= NLMISC::CFile::getFileModificationDate(DailyActivityLogFileName);
		}

		// if we already ran today then return
		if (startTime/(DailyTimeInterval) == lastTime/(DailyTimeInterval))
			return;

		// setup the new lastTime and oldJobsRemaining values
		uint32 oldJobsRemaining= CJobManager::getInstance()->getNumJobs();
		lastTime= (uint32)startTime;

		// execute daily tasks
		NLMISC::CConfigFile::CVar *commandsVar = NLNET::IService::getInstance()->ConfigFile.getVarPtr("DailyCommands");
		WARN_IF(commandsVar  == NULL,"'DailyCommands' not found in cfg file");
			
		// if we have daily commands...
		if (commandsVar!=NULL)
		{
			nlinfo("Executing Daily Commands");
			// iterate over the entries in the commandsVar, executing them 1 by 1
			for (uint i=0; i<commandsVar->size(); ++i)
			{
				NLMISC::CSString commandStr= commandsVar->asString(i);
				nlinfo("Executing daily command: %s",commandStr.strip().c_str());
				NLMISC::ICommand::execute(commandStr.strip(),*NLMISC::InfoLog);
			}
		}

		// if we've started jobs running then update the remaining jobs info
		uint32 newJobsRemaining= CJobManager::getInstance()->getNumJobs();
		if (oldJobsRemaining!=newJobsRemaining)
			jobsRemaining= newJobsRemaining;

		// get the end time
		time_t endTime;
		time( &endTime );

		// determine date info from time info
		tm * ptm;
		ptm = gmtime(&endTime);

		// write to the log file
		FILE* fileHandle= fopen(DailyActivityLogFileName,"ab");
		nlassert(fileHandle!=NULL);
		fprintf(fileHandle,"%02u/%02u/%u CDailyTaskScheduler: Started: %02u:%02u, Finished: %02u:%02u, Executed %u commands Started %u Jobs\n",
			ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, (uint)startTime/3600%24, (uint)startTime/60%60, (uint)endTime/3600%24, (uint)endTime/60%60, commandsVar==NULL?0:commandsVar->size(), jobsRemaining );
		nlinfo("JobManager state: %s",CJobManager::getInstance()->getStatus().c_str());
		fclose(fileHandle);
	}
};

static CDailyTaskScheduler dailyTaskScheduler;
