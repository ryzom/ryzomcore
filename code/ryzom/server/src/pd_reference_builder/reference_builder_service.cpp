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

#include "reference_builder_service.h"
#include <nel/net/unified_network.h>

#include "ref_builder_task.h"
#include "delta_builder_task.h"

#ifdef NL_OS_WINDOWS
#	define NOMINMAX
#	include <windows.h>
#endif // NL_OS_WINDOWS

using namespace std;
using namespace NLMISC;
using namespace NLNET;


void	onServiceDown(const std::string &serviceName, TServiceId sid, void *arg);
void	cbGenerateReference(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId);
void	cbGenerateDelta(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId);
void	cbKillTasks(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId);


//-----------------------------------------------
//	callback table for input message 
//
//-----------------------------------------------
TUnifiedCallbackItem CbArray[] =
{
	{ "RB_GEN_REF",		cbGenerateReference },
	{ "RB_GEN_DELTA",	cbGenerateDelta },
	{ "RB_KILL_TASKS",	cbKillTasks },
};












/*
 * Constructor
 */
CReferenceBuilderService::CReferenceBuilderService()
{
	CurrentTask = NULL;
}


//-----------------------------------------------
//	NLNET_SERVICE_MAIN
//-----------------------------------------------
NLNET_SERVICE_MAIN( CReferenceBuilderService, "RBS", "reference_builder_service", 0, CbArray, "", "" );





/*
 * Initialization
 */
void	CReferenceBuilderService::init()
{
	CUnifiedNetwork::getInstance()->setServiceDownCallback("*", onServiceDown);
}


/*
 * Release
 */
void	CReferenceBuilderService::release()
{
}




/*
 * Update
 */
bool	CReferenceBuilderService::update()
{
	// if current task is completed, delete it
	if (CurrentTask != NULL && CurrentTask->State == IRefTask::Completed)
	{
		// report execution result to requester
		if (CurrentTask->ExecutionSuccess)
		{
			CMessage	msgsuccess("RB_TASK_SUCCESS");
			msgsuccess.serial(CurrentTask->TaskId);
			CUnifiedNetwork::getInstance()->send(CurrentTask->RequesterService, msgsuccess);
		}
		else
		{
			CMessage	msgfailed("RB_TASK_FAILED");
			msgfailed.serial(CurrentTask->TaskId);
			CUnifiedNetwork::getInstance()->send(CurrentTask->RequesterService, msgfailed);
		}

		delete CurrentTask;
		CurrentTask = NULL;
	}

	// if no current task, peek another one
	if (CurrentTask == NULL && !Tasks.empty())
	{
		// pop new one
		CurrentTask = Tasks.front();
		Tasks.pop_front();

		// start it
		CurrentTask->start();
	}

	return true;
}


NLMISC_COMMAND(testNextTimestamp, "...", "timestamp")
{
	if (args.size() != 1)
		return false;

	string	ret = CRefBuilderTask::getNextTimestamp(args[0]);
	log.displayNL("%s", ret.c_str());
	return !ret.empty();
}










/*
 * Connection/Deconnection Handling
 */

void	onServiceDown(const std::string &serviceName, TServiceId sid, void *arg)
{
	CReferenceBuilderService*	service = (CReferenceBuilderService*)IService::getInstance();
	service->killTasks(sid);
}




/*
 * Messages Handling
 */

/**
 * Generate a new reference
 * Message RB_GEN_REF
 * Contains:
 * uint32	taskId					Id of the task, to receive success/failure notification
 * string	rootRefPath				Root path of the database
 * string	previousReferencePath	Path of the previous database reference files
 * string	nextReferencePath		Path of the next database reference files to be written
 * string	hoursUpdatePath			Path to the hour update files
 * string	minutesUpdatePath		Path to the minute update files
 * string	secondsUpdatePath		Path to the second update files
 * string	mintimestamp			Minimum timestamp of delta to use
 * string	maxtimestamp			Maximum timestamp of delta to use
 * string	deleteDeltaOlderThan	Minimum timestamp of delta files to keep
 */
void	cbGenerateReference(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	taskId;
	string	rootRefPath;
	string	previousReferencePath;
	string	nextReferencePath;
	string	hoursUpdatePath;
	string	minutesUpdatePath;
	string	secondsUpdatePath;
	string	logUpdatePath;
	string	mintimestamp;
	string	maxtimestamp;
	string	deleteDeltaOlderThan;

	msgin.serial(taskId);

	msgin.serial(rootRefPath);
	msgin.serial(previousReferencePath);
	msgin.serial(nextReferencePath);
	msgin.serial(hoursUpdatePath);
	msgin.serial(minutesUpdatePath);
	msgin.serial(secondsUpdatePath);
	msgin.serial(logUpdatePath);
	msgin.serial(mintimestamp);
	msgin.serial(maxtimestamp);
	msgin.serial(deleteDeltaOlderThan);

	CRefBuilderTask*	task = new CRefBuilderTask();

	task->TaskId = taskId;
	task->RequesterService = serviceId;

	task->setup(rootRefPath,
				previousReferencePath,
				nextReferencePath,
				hoursUpdatePath,
				minutesUpdatePath,
				secondsUpdatePath,
				logUpdatePath,
				mintimestamp,
				maxtimestamp,
				deleteDeltaOlderThan);

	CReferenceBuilderService*	service = (CReferenceBuilderService*)IService::getInstance();

	service->Tasks.push_back(task);
}



/**
 * Generate a delta update from other delta updates
 * Message RB_GEN_DELTA
 * Contains:
 * uint32	taskId					Id of the task, to receive success/failure notification
 * string	outputPath				Path to generate the delta update
 * string	hoursUpdatePath			Path to the hour update files
 * string	minutesUpdatePath		Path to the minute update files
 * string	secondsUpdatePath		Path to the second update files
 * string	mintimestamp			Minimum timestamp of delta to use
 * string	maxtimestamp			Maximum timestamp of delta to use
 * CDeltaBuilder::TDelta	type	Type of delta update (Minute, Hour)
 * string	deleteDeltaOlderThan	Minimum timestamp of delta files to keep
 */
void	cbGenerateDelta(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	uint32	taskId;
	string	outputPath;
	string	hoursUpdatePath;
	string	minutesUpdatePath;
	string	secondsUpdatePath;
	string	mintimestamp;
	string	maxtimestamp;
	CDeltaBuilder::TDelta	type;
	string	deleteDeltaOlderThan;

	msgin.serial(taskId);

	msgin.serial(outputPath);
	msgin.serial(hoursUpdatePath);
	msgin.serial(minutesUpdatePath);
	msgin.serial(secondsUpdatePath);
	msgin.serial(mintimestamp);
	msgin.serial(maxtimestamp);
	msgin.serialEnum(type);
	msgin.serial(deleteDeltaOlderThan);

	CDeltaBuilderTask*	task = new CDeltaBuilderTask();

	task->TaskId = taskId;
	task->RequesterService = serviceId;

	task->setup(outputPath,
				hoursUpdatePath,
				minutesUpdatePath,
				secondsUpdatePath,
				mintimestamp,
				maxtimestamp,
				type,
				deleteDeltaOlderThan);

	CReferenceBuilderService*	service = (CReferenceBuilderService*)IService::getInstance();

	service->Tasks.push_back(task);
}


/**
 * Ask RBS to Kill all requested tasks
 */
void	cbKillTasks(NLNET::CMessage& msgin, const std::string &serviceName, TServiceId serviceId)
{
	CReferenceBuilderService*	service = (CReferenceBuilderService*)IService::getInstance();
	service->killTasks(serviceId);
}



/*
 * Kill All Submitted Task By A Service
 */
void	CReferenceBuilderService::killTasks(TServiceId serviceId)
{
	// if current task is owned by disconnecting service, ask it to stop
	if (CurrentTask != NULL && CurrentTask->RequesterService == serviceId)
	{
		CurrentTask->askStop();
	}

	// remove all awaiting tasks this service requested
	std::deque<IRefTask*>::iterator	it;
	for (it=Tasks.begin(); it!=Tasks.end(); )
	{
		IRefTask*	task = (*it);

		if (task->RequesterService == serviceId)
			it = Tasks.erase(it);
		else
			++it;
	}
}




/*
 * 
 */

void	IRefTask::start()
{
	_Thread = IThread::create(this);
	_Thread->start();
}

void	IRefTask::run()
{
	State = Running;
	ExecutionSuccess = execute();
	State = Completed;
}

