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

#include "job_manager.h"

class CFinishedJob: public CJobManager::IJob
{
public:
	bool finished()									{ return true; }
	std::string getShortStatus()					{ return _ShortStatus; }
	std::string getStatus()							{ return _Status; }
	void display(NLMISC::CLog* log=NLMISC::InfoLog) { log->displayNL("%s",_Status.c_str()); }
	void update()									{}

	CFinishedJob(CJobManager::IJob* theFinishedJob)
	{
		if (theFinishedJob==NULL)
			return;
		_Status=theFinishedJob->getStatus();
		_ShortStatus=theFinishedJob->getShortStatus();
	}

private:
	std::string _Status;
	std::string _ShortStatus;

};

CJobManager* CJobManager::getInstance()
{
	static CJobManager* mgr=NULL;
	if (mgr==NULL)
	{
		mgr=new CJobManager;
	}
	return mgr;
}

CJobManager::CJobManager()
{
	_Paused=false;
	_JobUpdatesPerUpdate=1;
}

void CJobManager::serviceUpdate()
{
	if (_Paused)
		return;

	for (uint32 count=0;count<_JobUpdatesPerUpdate &&!_UnfinishedJobs.empty();++count)
	{
		nlassert(_UnfinishedJobs.front()<_Jobs.size());
		NLMISC::CSmartPtr<IJob>& theJob= _Jobs[_UnfinishedJobs.front()];
		if (theJob->finished())
		{
			// delete the job and replace it with a light weight 'finished job' marker
			theJob= new CFinishedJob(theJob);

			// remove this job from the list of unfinished jobs
			_UnfinishedJobs.pop_front();

			// decrement the updates counter to counteract the auto incrment
			--count;
		}
		else
		{
			theJob->update();
		}
	}
}


uint32 CJobManager::addJob(NLMISC::CSmartPtr<CJobManager::IJob> job)
{
	nlassert(job!=NULL);
	uint32 id= (uint32)_Jobs.size();
	_UnfinishedJobs.push_back(id);
	_Jobs.push_back(job);
	return id;
}

void CJobManager::promoteJob(uint32 idx)
{
	TUnfinishedJobs::iterator it;
	for (it=_UnfinishedJobs.begin(); it!=_UnfinishedJobs.end(); ++it)
	{
		if (*it==idx)
		{
			_UnfinishedJobs.erase(it);
			_UnfinishedJobs.push_front(idx);
			return;
		}
	}
	nlwarning("Failed to promote job with ID %d as not found in unfinished jobs list",idx);
}

void CJobManager::pause()
{
	_Paused= true;
}

void CJobManager::resume()
{
	_Paused= false;
}

void CJobManager::setJobUpdatesPerUpdate(uint32 count)
{
	_JobUpdatesPerUpdate= count;
	if (count==0 || count>100)
		nlwarning("Suspicious value of JobUpdatesPerUpdate: %d",count);
}

uint32 CJobManager::getJobUpdatesPerUpdate()
{
	return _JobUpdatesPerUpdate;
}

std::string CJobManager::getStatus()
{
	std::string result;

	if (_Paused) result+="[Paused] ";
	
	if (!_UnfinishedJobs.empty())
	{
		uint32 idx=_UnfinishedJobs.front();
		nlassert(idx<_Jobs.size());
		result+=_Jobs[idx]->getStatus();
	}

	result+=NLMISC::toString(" [Updates per cycle: %d]",_JobUpdatesPerUpdate);

	return result;
}

void CJobManager::listJobs(NLMISC::CLog* log)
{
	for (uint32 i=0;i< _Jobs.size(); ++i)
	{
		if (!_Jobs[i]->finished())
			nlinfo("%4d*: %s",i,_Jobs[i]->getStatus().c_str());
	}
	nlinfo("%d unfinished jobs (%d  in total)",_UnfinishedJobs.size(),_Jobs.size());
}

void CJobManager::listJobHistory(NLMISC::CLog* log)
{
	for (uint32 i=0;i< _Jobs.size(); ++i)
	{
		nlinfo("%4d%c: %s",i,_Jobs[i]->finished()? ' ': '*',_Jobs[i]->getStatus().c_str());
	}
	nlinfo("%d unfinished jobs (%d  in total)",_UnfinishedJobs.size(),_Jobs.size());
}

void CJobManager::displayCurrentJob(NLMISC::CLog* log)
{
	if (!_UnfinishedJobs.empty())
		displayJob(_UnfinishedJobs.front(),log);
}

void CJobManager::displayJob(uint32 jobId,NLMISC::CLog* log)
{
	nlassert(jobId<_Jobs.size());
	_Jobs[jobId]->display(log);
}
