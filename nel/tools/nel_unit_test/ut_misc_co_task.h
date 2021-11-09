// NeL - MMORPG Framework <http://dev.ryzom.com/projects/nel/>
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

#ifndef UT_MISC_CO_TASK
#define UT_MISC_CO_TASK

#include <nel/misc/co_task.h>
#include <nel/misc/thread.h>

const char *referenceResult[] =
{
	"Task1 : 0",
	"Task2 : 0",
	"Main : 0",

	"Task1 : 1",
	"Task2 : 1",
	"Main : 1",

	"Task1 : 2",
	"Task2 : 2",

	"Task1 : 3",
	"Task2 : 3",

	"Task1 : 4",
	"Task2 : 4",

	"Task2 : 5",

	"Task2 : 6",
};

const char *referenceResultThread1[] =
{
	"Task1 : 0",
	"Thread : 0",

	"Task1 : 1",
	"Thread : 1",

	"Task1 : 2",
	"Thread : 2",

	"Task1 : 3",
	"Thread : 3",

	"Task1 : 4",
	"Thread : 4",
};

const char *referenceResultThread2[] =
{
	"Task2 : 0",
	"Main : 0",

	"Task2 : 1",
	"Main : 1",

	"Task2 : 2",

	"Task2 : 3",

	"Task2 : 4",
};

vector<string>	result;
vector<string>	result2;

// a simple task
class CTask1 : public NLMISC::CCoTask
{
	vector<string>	&Output;
public:
	CTask1(vector<string> &output = result)
		: Output(output)
	{}

	void run()
	{
		for (uint i=0; i<5; ++i)
		{
			string s = NLMISC::toString("Task1 : %u", i);
			Output.push_back(s);
			yield();
		}
	}
};

// another simple task
class CTask2 : public NLMISC::CCoTask
{
	vector<string>	&Output;

public:
		CTask2(vector<string> &output = result)
		: Output(output)
	{}

	void run()
	{
		for (uint i=0; i<7; ++i)
		{
			string s = NLMISC::toString("Task2 : %u", i);
			Output.push_back(s);
			yield();
		}
	}
};

// a thread runnable class
class CTaskThread : public NLMISC::IRunnable
{
	void run()
	{
		CTask1	t1(result2);

		for (uint i=0; i<5; ++i)
		{
			t1.resume();
			string s = NLMISC::toString("Thread : %u", i);
			result2.push_back(s);
			NLMISC::nlSleep(0);
		}
	}
};

// Test suite for coroutine task
class CUTMiscCoTask: public Test::Suite
{
public:
	CUTMiscCoTask()
	{
		TEST_ADD(CUTMiscCoTask::runTasks);
		TEST_ADD(CUTMiscCoTask::tasksAndThreads);

	}

	void tasksAndThreads()
	{
		// test running task in two separate thread (this stress the 
		// multithreading support of task). CoTask API ;ake use of
		// thread local storage API to store by thread current task info.

		result.clear();
		result2.clear();

		CTaskThread	tt;
		NLMISC::IThread *th = NLMISC::IThread::create(&tt);

		CTask2	t2;

		// start the thread
		th->start();

		for (uint i=0; i<2; ++i)
		{
			t2.resume();
			string s = NLMISC::toString("Main : %u", i);
			result.push_back(s);
			NLMISC::nlSleep(0);
		}

		// wait task completion
		t2.wait();

		// wait thread completion
		th->wait();

		delete th;

		// test result
		for (uint i=0; i<sizeofarray(referenceResultThread1); ++i)
		{
			string &s1 = result2[i];
			const char *s2 = referenceResultThread1[i];
			TEST_ASSERT(referenceResultThread1[i] == result2[i]);
		}
		for (uint i=0; i<sizeofarray(referenceResultThread2); ++i)
		{
			string &s1 = result[i];
			const char *s2 = referenceResultThread2[i];
			TEST_ASSERT(referenceResultThread2[i] == result[i]);
		}
	}
	
	void runTasks()
	{
		/// Run two main task and two working task at once and check that the result
		result.clear();

		// create the two task object
		CTask1 t1;
		CTask2 t2;

		// start the two task (can be done in the loop, but we want the task output first)
		t1.resume();
		t2.resume();

		// loop and run the main task and the two sub task
		for (uint i=0; i<2; ++i)
		{
			string s = NLMISC::toString("Main : %u", i);
			result.push_back(s);
			t1.resume();
			t2.resume();
		}

		// continue to run the task until the first is terminated
		while (!t1.isFinished())
		{
			t1.resume();
			t2.resume();
		}

		// wait task termination
		t1.wait();
		t2.wait();

		// check the generated result
		TEST_ASSERT(sizeofarray(referenceResult) == result.size());
		for (uint i=0; i<sizeofarray(referenceResult); ++i)
		{
			string &s1 = result[i];
			const char *s2 = referenceResult[i];
			TEST_ASSERT(referenceResult[i] == result[i]);
		}
	}
};

#endif
