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
#include "mutex.h"

struct SDL_Thread;

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

	CThread *thread = new CThread (new HelloLoopThread);
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
	virtual void getName(std::string &result) const
	{
		result = "NoName";
	}
};

/// Thread priorities, numbering follows Win32 for now
enum TThreadPriority
{
	ThreadPriorityLow,
	ThreadPriorityNormal,
	ThreadPriorityHigh
};

/**
 * Interface to SDL2 threads
 * \date 2014
 */
class CThread
{
public:
	CThread(IRunnable *runnable);

	// Thread will be detached immediately and remain running.
	~CThread();

	// Starts the thread.
	void start();

	// Check if the thread is still running
	bool isRunning();

	/// In the calling program, wait until the specified thread has exited.
	int wait();

	/// Return a pointer to the runnable object
	IRunnable *getRunnable();

	/// Set the priority for the current thread.
	static void setPriority(TThreadPriority priority);

private:
	static int run(void *ptr);

	volatile bool m_IsRunning;
	SDL_Thread *m_SDLThread;
	IRunnable *m_Runnable;
	CMutex m_WaitMutex; // Mutex to allow multiple threads to wait for one thread
	int m_ThreadResult; // Result from last thread run

};

/*
 * Thread exception
 */
struct EThread : public Exception
{
	EThread (const char* message) : Exception (message) {}
};

} // NLMISC

#endif // NL_THREAD_H

/* End of thread.h */
