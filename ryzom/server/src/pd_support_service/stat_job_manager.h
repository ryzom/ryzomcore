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

#ifndef STAT_JOB_MANAGER_H
#define STAT_JOB_MANAGER_H

#include "nel/misc/types_nl.h"
#include "nel/misc/debug.h"
#include "nel/misc/smart_ptr.h"
#include "game_share/singleton_registry.h"

class CJobManager: public IServiceSingleton
{
public:
	static CJobManager* getInstance();

public:
	class IJob: public NLMISC::CRefCount
	{
	public:
		// virtual dtor
		virtual ~IJob() {}

		// start a job running (one that isn't in progress)
		virtual void start()=0;

		// return true if the job is finished -> the job object can be deleted
		virtual bool finished()=0;

		// return a status string that can be displayed in an update variable
		virtual std::string getShortStatus()=0;

		// get a detailed status string for display in a job list etc
		virtual std::string getStatus()=0;

		// display the details of the job... eg the list of files being processed with related info
		virtual void display(NLMISC::CLog* log=NLMISC::InfoLog)=0;

		// run the job's update to do a bit of work
		virtual void update()=0;
	};

public:
	// add a job to the job vector and assign it a new id
	uint32 addJob(NLMISC::CSmartPtr<IJob> job);

	// move a job to the front of the queue - treat it as the active job
	void promoteJob(uint32 idx);

	// the update method used to call job updates
	void serviceUpdate();

	// do nothing during the service updates until 'resume()'
	void pause();

	// resume after a 'pause()'
	void resume();

	// accessors for the number of job updates per call to serviceUpdate()
	void setJobUpdatesPerUpdate(uint32 count);
	uint32 getJobUpdatesPerUpdate();

	// get the 'pause'/'resume' state, the number of jobsUpdatesPerUpdate and the status of the active job
	std::string getStatus();

	// list the status of all jobs that are not finished
	void listJobs(NLMISC::CLog* log=NLMISC::InfoLog);

	// list the status of all jobs including those that are finished
	void listJobHistory(NLMISC::CLog* log=NLMISC::InfoLog);

	// call the currently active job's 'display' method
	void displayCurrentJob(NLMISC::CLog* log=NLMISC::InfoLog);

	// call the given job's 'display' method
	void displayJob(uint32 jobId,NLMISC::CLog* log=NLMISC::InfoLog);

	// returns the number of unfinished jobs in the manager
	uint32 getNumJobs() const;

private:
	CJobManager();

	typedef std::list<uint32> TUnfinishedJobs;
	typedef std::vector<NLMISC::CSmartPtr<IJob> > TJobs;

	bool _Paused;
	uint32 _JobUpdatesPerUpdate;
	TJobs _Jobs;
	TUnfinishedJobs _UnfinishedJobs;
};

#endif
