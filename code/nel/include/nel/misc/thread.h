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

#ifndef NL_THREAD_H
#define NL_THREAD_H

#include "types_nl.h"
#include "common.h"


namespace NLMISC {


/**
 * Thread callback interface.
 * When a thread is created, it will call run() in its attached IRunnable interface.
 *
 *\code

	#include "thread.h"

	class HelloLoopThread : public IRunnable
	{
		void run ()
		{
			for(;;)	printf("Hello World\n");
		}

	};

	IThread *thread = IThread::create (new HelloLoopThread);
	thread->start ();

 *\endcode
 *
 *
 *
 *
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IRunnable
{
public:
	// Called when a thread is run.
	virtual void run()=0;
	virtual ~IRunnable()
	{
	}
	// Return the runnable name
	virtual void getName (std::string &result) const
	{
		result = "NoName";
	}
};

/// Thread priorities, numbering follows Win32 for now
enum TThreadPriority
{
	ThreadPriorityLowest = -2,
	ThreadPriorityLow = -1,
	ThreadPriorityNormal = 0,
	ThreadPriorityHigh = 1,
	ThreadPriorityHighest = 2,
};

/**
 * Thread base interface, must be implemented for all OS
 * \author Vianney Lecroart
 * \author Nevrax France
 * \date 2000
 */
class IThread
{
public:

	/**
	  * Create a new thread.
	  * Implemented in the derived class.
	  */
	static IThread *create(IRunnable *runnable, uint32 stackSize = 0);

	/**
	  * Return a pointer on the current thread.
	  * Implemented in the derived class.
	  */
	static IThread *getCurrentThread ();

	virtual ~IThread () { }

	// Starts the thread.
	virtual void start()=0;

	// Check if the thread is still running
	virtual bool isRunning() =0;

	// Terminate the thread (risky method, use only in extreme cases)
	virtual void terminate()=0;

	// In the calling program, wait until the specified thread has exited. After wait() has returned, you can delete the thread object.
	virtual void wait()=0;

	/// Return a pointer to the runnable object
	virtual IRunnable *getRunnable()=0;

	/**
	  * Set the CPU mask of this thread. Thread must have been started before.
	  * The mask must be a subset of the CPU mask returned by IProcess::getCPUMask() thread process.
	  */
	virtual bool setCPUMask(uint64 cpuMask)=0;

	/**
	  * Get the CPU mask of this thread. Thread must have been started before.
	  * The mask should be a subset of the CPU mask returned by IProcess::getCPUMask() thread process.
	  */
	virtual uint64 getCPUMask()=0;

	/// Set the thread priority. Thread must have been started before.
	virtual void setPriority(TThreadPriority priority) = 0;

	/**
	  * Get the thread user name.
	  * Under Linux return thread owner, under windows return the name of the logon user.
	  */
	virtual std::string getUserName()=0;
};


/*
 * Thread exception
 */
struct EThread : public Exception
{
	EThread (const char* message) : Exception (message) {}
};


/**
 * Process base interface, must be implemented for all OS
 * \author Cyril 'Hulud' Corvazier
 * \author Nevrax France
 * \date 2000
 */
class IProcess
{
public:
	virtual ~IProcess() {}

	/**
	  * Return a pointer on the current process.
	  * Implemented in the derived class.
	  */
	static IProcess *getCurrentProcess ();

	/**
	  * Return process CPU mask. Each bit stand for a CPU usable by the process threads.
	  */
	virtual uint64 getCPUMask()=0;

	/**
	  * Set the process CPU mask. Each bit stand for a CPU usable by the process threads.
	  */
	virtual bool setCPUMask(uint64 mask)=0;
};


} // NLMISC


#endif // NL_THREAD_H

/* End of thread.h */
