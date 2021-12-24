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

#ifndef SERVICE_MAIN_H
#define SERVICE_MAIN_H


//-----------------------------------------------------------------------------
// includes
//-----------------------------------------------------------------------------

#include "nel/net/service.h"


//-----------------------------------------------------------------------------
// class CServiceClass 
//-----------------------------------------------------------------------------

class CServiceClass : public NLNET::IService
{
public:
	// service basics
	void init();
	bool update();
	void release();

private:
	// private data
	bool _ExitRequested;
};


//-----------------------------------------------------------------------------
// class IScheduledTask
//-----------------------------------------------------------------------------

class IScheduledTask: public NLMISC::CRefCount
{
public:
	//-------------------------------------------------------------------------
	// virtual dtor...
	
	virtual ~IScheduledTask() {}


	//-------------------------------------------------------------------------
	// routine that does whatever work this scheduled task is designed for

	virtual void execute() =0;
};


//-----------------------------------------------------------------------------
// class CTaskScheduler
//-----------------------------------------------------------------------------

class CTaskScheduler
{
public:
	// getter for the singleton instance
	static CTaskScheduler& getInstance();

	// registration method (task and delay in miliseconds before execution)
	void sceduleTask(IScheduledTask* task,NLMISC::TTime delay);

	// test method - returns the number of errors found
	void update();

private:
	// prohibit instantiation because we're a singleton
	CTaskScheduler();

	// private data
	typedef NLMISC::CSmartPtr<IScheduledTask> TScheduledTaskSmartPtr;
	struct STask
	{
		TScheduledTaskSmartPtr	TaskPtr;
		NLMISC::TTime			ExecutionTime;
	};
	typedef std::vector<STask> TTasks;
	TTasks _Tasks;		// the current task vector

	uint32 _MaxTasks;	// the largest number of tasks that we have had in the task vector at any one time
};


//-----------------------------------------------------------------------------
// class ISelfTestClass
//-----------------------------------------------------------------------------

class ISelfTestClass: public NLMISC::CRefCount
{
public:
	//-------------------------------------------------------------------------
	// make the dtor virtual...
	
	virtual ~ISelfTestClass() {}


	//-------------------------------------------------------------------------
	// routine that runs the test - returns _ErrorCount

	virtual uint32 runTest() =0;


	//-------------------------------------------------------------------------
	// init

	void init(NLMISC::CLog& errorLog, NLMISC::CLog& progressLog, NLMISC::CLog& verboseLog);

	
	//-------------------------------------------------------------------------
	// logging

	void errorLog(const char* fmt,...) const;
	void progressLog(const char* fmt,...) const;
	void verboseLog(const char* fmt,...) const;


	//-------------------------------------------------------------------------
	// error counter

	void addError() const;
	uint32 getErrorCount() const;


protected:
	//-------------------------------------------------------------------------
	// protected data

	mutable NLMISC::CLog*	_ErrorLog;
	mutable NLMISC::CLog*	_ProgressLog;
	mutable NLMISC::CLog*	_VerboseLog;
	mutable uint32			_ErrorCount;
};


//-----------------------------------------------------------------------------
// class CSelfTestRegistry
//-----------------------------------------------------------------------------

class CSelfTestRegistry
{
public:
	// getter for the singleton instance
	static CSelfTestRegistry& getInstance();

	// registration method
	void registerTestObject(ISelfTestClass* testObject,const NLMISC::CSString& description);

	// test method - returns the number of errors found
	uint32 runTests(NLMISC::CLog& errorLog, NLMISC::CLog& progressLog, NLMISC::CLog& verboseLog);

private:
	// prohibit instantiation because we're a singleton
	CSelfTestRegistry();

	// private data
	typedef NLMISC::CSmartPtr<ISelfTestClass> TSelfTestClassSmartPtr;
	typedef std::map<TSelfTestClassSmartPtr,NLMISC::CSString> TTests; // map from test object pointer to description
	TTests _Tests;
};


//-----------------------------------------------------------------------------
// macro REGISTER_SELF_TEST
//-----------------------------------------------------------------------------

#define REGISTER_SELF_TEST(CLASS,DESCRIPTION)\
	struct CSelfTestRegisterer_##CLASS\
	{\
		CSelfTestRegisterer_##CLASS() { CSelfTestRegistry::getInstance().registerTestObject(new CLASS,DESCRIPTION); }\
	} selfTestRegisterer_##CLASS;\


//-----------------------------------------------------------------------------
#endif
