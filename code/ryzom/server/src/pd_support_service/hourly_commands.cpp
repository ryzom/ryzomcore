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

NLMISC::CVariable<std::string> HourlyActivityLogFileName("scheduler", "HourlyActivityLogFileName", "log file name for hourly activity logs", "pdss_hourly_activity.log", 0, true);
NLMISC::CVariable<uint32> HourlyTimeInterval("scheduler", "HourlyTimeInterval", "number of seconds in an hour", 60*60, 0, true);
NLMISC::CVariable<uint32> HourlyStartTime("scheduler", "HourlyStartTime", "number of seconds into the hour when hourly tasks should be launched", 15*60, 0, true);

class CHourlyTaskScheduler: public IServiceSingleton
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

		// if we're too early in the hour then return
		if ( uint32(startTime%HourlyTimeInterval.get()) <= HourlyStartTime )
			return;

		// setup the lastTime corresponding to the last time we ran
		static uint32 lastTime=0;
		if (lastTime==0 && NLMISC::CFile::fileExists(HourlyActivityLogFileName))
		{
			lastTime= NLMISC::CFile::getFileModificationDate(HourlyActivityLogFileName);
		}

		// if we already ran within the last hour then return
		if (startTime/(HourlyTimeInterval) == lastTime/(HourlyTimeInterval))
			return;

		// setup the new lastTime and oldJobsRemaining values
		uint32 oldJobsRemaining= CJobManager::getInstance()->getNumJobs();
		lastTime= (uint32)startTime;

		// execute hourly tasks
		NLMISC::CConfigFile::CVar *commandsVar = NLNET::IService::getInstance()->ConfigFile.getVarPtr("HourlyCommands");
		WARN_IF(commandsVar  == NULL,"'HourlyCommands' not found in cfg file");

		// if we have hourly commands...
		if (commandsVar!=NULL)
		{
			nlinfo("Executing Hourly Commands");
			// iterate over the entries in the commandsVar, executing them 1 by 1
			for (uint i=0; commandsVar!=NULL && i<commandsVar->size(); ++i)
			{
				NLMISC::CSString commandStr= commandsVar->asString(i);
				nlinfo("Executing hourly command: %s",commandStr.strip().c_str());
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
		FILE* fileHandle= fopen(HourlyActivityLogFileName,"ab");
		nlassert(fileHandle!=NULL);
		fprintf(fileHandle,"%02u/%02u/%u CHourlyTaskScheduler: Started: %02u:%02u, Finished: %02u:%02u, Executed %u commands Started %u Jobs\n",
			ptm->tm_mday, ptm->tm_mon+1, ptm->tm_year+1900, (uint)startTime/3600%24, (uint)startTime/60%60, (uint)endTime/3600%24, (uint)endTime/60%60, commandsVar==NULL?0:commandsVar->size(), jobsRemaining );
		nlinfo("JobManager state: %s",CJobManager::getInstance()->getStatus().c_str());
		fclose(fileHandle);
	}
};

static CHourlyTaskScheduler HourlyTaskScheduler;
